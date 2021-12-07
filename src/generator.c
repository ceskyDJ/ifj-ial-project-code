/*
 * @file generator.c
 * Generator implementation.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#include "exit_codes.h"
#include "generator.h"
#include "logger.h"
#include "symqueue.h"
#include "token.h"

#include <stdio.h>

static void print_function_signature(identifier_t *id)
{
    if (!id->fun.param) {
        return;
    }
    printf("# %s(", id->name);
    for (int i = 0; id->fun.param[i] != '\0'; i++)
        printf("%s%c", i == 0 ? "" : ",", id->fun.param[i]);
    printf(")\n");
}

static unsigned int fun_param_cnt = 1;
static unsigned int call_param_cnt = 1;
static unsigned int retval_cnt = 1;
static unsigned int var_cnt = 1;
static unsigned int if_cnt = 1;
static unsigned int while_cnt = 1;
static unsigned int nil_check_cnt = 1;
static unsigned int zero_div_check_cnt = 1;
static unsigned int return_assign_cnt = 1;

void gen_builtins(void)
{
    // TODO
}

void gen_ifjcode21(void)
{
    printf(".IFJcode21\n");
}

void gen_fun_start(identifier_t *id)
{
    print_function_signature(id);
    printf("JUMP &%s_skip\n", id->name);
    printf("LABEL &%s\n", id->name);
    printf("PUSHFRAME\n");
    // tmp variables used for string operations, LEQ, GEQ and zero_div_check
    printf("DEFVAR LF@$op_tmp_1\n");
    printf("DEFVAR LF@$op_tmp_2\n");

    fun_param_cnt = 1;
    retval_cnt = 1;
}

void gen_fun_param(identifier_t *id)
{
    // will be called for each encountered param
    // need to move value that was passed to us to it
    printf("DEFVAR LF@%s_%lu_%lu\n",
            id->name,
            id->line,
            id->character);
    printf("MOVE LF@%s_%lu_%lu LF@%%%u\n",
            id->name,
            id->line,
            id->character,
            fun_param_cnt);

    fun_param_cnt++;
}

void gen_create_frame()
{
    printf("CREATEFRAME\n");

    call_param_cnt = 1;
    return_assign_cnt = 1;
}

void gen_call_param(token_t *token)
{
    // when calling a function, prepare value for passing
    printf("DEFVAR TF@%%%u\n", call_param_cnt);

    if (token->type == IDENTIFIER) {
        printf("MOVE TF@%%%u LF@%s_%lu_%lu\n",
                call_param_cnt,
                token->identifier->name,
                token->identifier->line,
                token->identifier->character);
    } else if (token->type == KEYWORD) {
        if (*(token->keyword) == KW_NIL) {
            printf("MOVE TF@%%%u nil@nil\n",
                    call_param_cnt);
        }
    } else if (token->type == INTEGER) {
        printf("MOVE TF@%%%u int@%d\n",
                call_param_cnt,
                token->integer);
    } else if (token->type == NUMBER) {
        printf("MOVE TF@%%%u float@%a\n",
                call_param_cnt,
                token->number);
    } else if (token->type == STRING) {
        printf("MOVE TF@%%%u string@%s\n",
                call_param_cnt,
                token->string);
    }

    call_param_cnt++;
}

void gen_call(identifier_t *id)
{
    printf("CALL &%s\n", id->name);
}

void gen_returned_assign(symqueue_t *queue)
{
    identifier_t *var = symqueue_pop(queue);

    if (!var) {
        // this should never happen as this function should
        // be called only the correct amount of times,
        // when var in symqueue is guaranteed
        LOG_ERROR_M("Symqueue is empty");
        exit(EINTERNAL);
    }

    printf("MOVE LF@%s_%lu_%lu TF@%%retval_%d\n",
            var->name,
            var->line,
            var->character,
            return_assign_cnt);
    return_assign_cnt++;
}

// prepares variable for later assignment
void gen_var_dec(identifier_t *id, symqueue_t *queue)
{
    // TODO solve redefinition in while
    printf("DEFVAR LF@%s_%lu_%lu\n",
            id->name,
            id->line,
            id->character);

    symqueue_add(queue, id);
}

void gen_var_dec_assign(symqueue_t *queue, bool value_on_stack)
{
    identifier_t *var = symqueue_pop(queue);
    if (!var) {
        LOG_DEBUG_M("Symqueue is empty on var dec assign.");
        return;
    }
    if (value_on_stack) {
        // expr/call result is on stack
        // TODO what if function returned multiple values?
        // CLEARS after this?
        printf("POPS LF@%s_%lu_%lu\n",
                var->name,
                var->line,
                var->character);
    } else {
        // there is no expr result
        printf("MOVE LF@%s_%lu_%lu nil@nil\n",
                var->name,
                var->line,
                var->character);
    }
}

void gen_var_set_active(identifier_t *id, symqueue_t *queue)
{
    LOG_DEBUG("Add '%s' to symqueue as active variable", id->name);
    symqueue_add(queue, id);
}

void gen_var_active_assign(symqueue_t *queue, bool value_on_stack)
{
    identifier_t *var = symqueue_pop(queue);
    // wants to assign, no var in queue, drop the value, not an error
    if (!var) {
        LOG_DEBUG_M("Symqueue is empty on var active assign.");
        return;
    }

    if (value_on_stack) {
        // value is ready on top of the stack
        if (var->line == 0 && var->character == 0) {
            printf("POPS LF@%s\n", var->name);
        } else {
            printf("POPS LF@%s_%lu_%lu\n",
                    var->name,
                    var->line,
                    var->character);
        }
    } else {
        LOG_ERROR_M("Missing value on stack for active variable assignment. Should exit()");
    }
}

void gen_var_retval(void)
{
    printf("DEFVAR LF@%%retval_%u\n",
            retval_cnt);
    printf("MOVE LF@%%retval_%u nil@nil\n",
            retval_cnt);

    retval_cnt++;
}

void gen_fun_end(identifier_t *id)
{
    printf("POPFRAME\n");
    printf("RETURN\n");
    printf("LABEL &%s_skip\n", id->name);
}

void gen_push_term(token_t *token)
{
    if (token->type == IDENTIFIER) {
        printf("PUSHS LF@%s_%lu_%lu\n",
                token->identifier->name,
                token->identifier->line,
                token->identifier->character);
    } else if (token->type == KEYWORD) {
        if (*(token->keyword) == KW_NIL) {
            printf("PUSHS nil@nil\n");
        }
    } else if (token->type == INTEGER) {
        printf("PUSHS int@%d\n", token->integer);
    } else if (token->type == NUMBER) {
        printf("PUSHS float@%a\n", token->number);
    } else if (token->type == STRING) {
        printf("PUSHS string@%s\n", token->string);
    }
}

void gen_operation(enum token_type type)
{
    switch(type) {
        case MULTIPLICATION: printf("MULS\n"); break;
        case DIVISION: printf("DIVS\n"); break;
        case INT_DIVISION: printf("IDIVS\n"); break;
        case ADDITION: printf("ADDS\n"); break;
        case SUBTRACTION: printf("SUBS\n"); break;
        case LT: printf("LTS\n"); break;
        case GT: printf("GTS\n"); break;
        case EQ: printf("EQS\n"); break;
        case NEQ:
            printf("EQS\n");
            printf("NOTS\n");
            break;
        case LEQ:
            // pop in reverse order to get them correctly
            printf("POPS LF@$op_tmp_2\n");
            printf("POPS LF@$op_tmp_1\n");
            // LEQ --> NOT GT
            printf("GT LF@$op_tmp_1 LF@$op_tmp_1 LF@$op_tmp_2\n");
            printf("NOT LF@$op_tmp_1 LF@$op_tmp_1\n");
            printf("PUSHS LF@$op_tmp_1\n");
            break;
        case GEQ:
            // pop in reverse order to get them correctly
            printf("POPS LF@$op_tmp_2\n");
            printf("POPS LF@$op_tmp_1\n");
            // GEQ --> NOT LT
            printf("LT LF@$op_tmp_1 LF@$op_tmp_1 LF@$op_tmp_2\n");
            printf("NOT LF@$op_tmp_1 LF@$op_tmp_1\n");
            printf("PUSHS LF@$op_tmp_1\n");
            break;
        case STRLEN:
            printf("POPS LF@$op_tmp_1\n");
            printf("STRLEN LF@$op_tmp_1 LF@$op_tmp_1\n");
            printf("PUSHS LF@$op_tmp_1\n");
            break;
        case CONCAT:
            // pop strings in reverse order to get them correctly
            printf("POPS LF@$op_tmp_2\n");
            printf("POPS LF@$op_tmp_1\n");
            printf("CONCAT LF@$op_tmp_1 LF@$op_tmp_1 LF@$op_tmp_2\n");
            printf("PUSHS LF@$op_tmp_1\n");
            break;
        default:
            LOG_ERROR("Invalid operation token_type: %u", type);
            exit(EINTERNAL);
    }
}

unsigned int gen_if_start()
{
    if_cnt++;

    // expects expression result (even non bool) at the top of the stack
    printf("# IF <expr> THEN ...\n");
    printf("DEFVAR LF@$if_expr_%u\n", var_cnt);
    printf("POPS LF@$if_expr_%u\n", var_cnt);
    printf("PUSHS LF@$if_expr_%u\n", var_cnt);
    printf("TYPE LF@$if_expr_%u LF@$if_expr_%u\n", var_cnt, var_cnt);
    printf("JUMPIFNEQ &if_expr_not_bool_%u LF@$if_expr_%u string@bool\n", if_cnt, var_cnt);
    printf("# expr is bool and is on top of the stack\n");
    printf("PUSHS bool@false\n");
    printf("JUMPIFEQS &else_%u\n", if_cnt);
    printf("JUMP &then_%u\n", if_cnt);
    printf("LABEL &if_expr_not_bool_%u\n", if_cnt);
    // anything other than `nil` is considered true
    printf("# expr is NOT bool\n");
    printf("JUMPIFEQ &else_%u LF@$if_expr_%u string@nil\n", if_cnt, var_cnt);
    printf("LABEL &then_%u\n", if_cnt);
    printf("# code for THEN follows...\n");

    var_cnt++;

    return if_cnt;
}

void gen_if_else(unsigned int if_cnt)
{
    printf("# ELSE ...\n");
    printf("JUMP &end_if_%u\n", if_cnt);
    printf("LABEL &else_%u\n", if_cnt);
    printf("# code for ELSE follows...\n");
}

void gen_if_end(unsigned int if_cnt)
{
    printf("LABEL &end_if_%u\n", if_cnt);
}

void gen_while_start_before_expr()
{
    printf("# WHILE EXPR DO ...\n");
    printf("DEFVAR LF@$while_expr_%u\n", var_cnt);
    printf("LABEL &while_%u\n", while_cnt);
    printf("# code for EXPR follows...\n");
}

void gen_while_start_after_expr()
{
    printf("# expression result ready on top of the stack\n");
    printf("POPS LF@$while_expr_%u\n", var_cnt);
    printf("PUSHS LF@$while_expr_%u\n", var_cnt);
    printf("TYPE LF@$while_expr_%u LF@$while_expr_%u\n", var_cnt, var_cnt);
    printf("JUMPIFNEQ &while_expr_not_bool_%u LF@$while_expr_%u string@bool\n", while_cnt, var_cnt);
    printf("# expr is bool and is on top of the stack\n");
    printf("PUSHS bool@false\n");
    printf("JUMPIFEQS &while_end_%u\n", while_cnt);
    printf("JUMP &while_do_%u\n", while_cnt);
    printf("LABEL &while_expr_not_bool_%u\n", while_cnt);
    // anything other than `nil` is considered true
    printf("# expr is NOT bool\n");
    printf("JUMPIFEQ &while_end_%u LF@$while_expr_%u string@nil\n", while_cnt, var_cnt);
    printf("LABEL &while_do_%u\n", while_cnt);
    printf("# code for DO follows\n");

    var_cnt++;
}    

void gen_while_end()
{
    printf("JUMP &while_%u\n", while_cnt);
    printf("LABEL &while_end_%u\n", while_cnt);

    while_cnt++;
}

void gen_nil_check_top()
{
    printf("# Nil check for top value\n");
    // save top value
    printf("POPS LF@$op_tmp_1\n");
    // compare it to nil
    printf("JUMPIFNEQ &nil_check_ok_%u LF@$op_tmp_1 nil@nil\n", nil_check_cnt);
    printf("EXIT int@%u\n", ENIL);
    printf("LABEL &nil_check_ok_%u\n", nil_check_cnt);
    // restore top
    printf("PUSHS LF@$op_tmp_1\n");

    nil_check_cnt++;
}

void gen_nil_check_subtop()
{
    printf("# Nil check for subtop value\n");
    // save top value on stack
    printf("POPS LF@$op_tmp_1\n");
    printf("POPS LF@$op_tmp_2\n");
    // compare to nil
    printf("JUMPIFNEQ &nil_check_ok_%u LF@$op_tmp_2 nil@nil\n", nil_check_cnt);
    printf("EXIT int@%u\n", ENIL);
    printf("LABEL &nil_check_ok_%u\n", nil_check_cnt);
    // restore both values
    printf("PUSHS LF@$op_tmp_2\n");
    printf("PUSHS LF@$op_tmp_1\n");
    nil_check_cnt++;
}

void gen_zero_div_check()
{
    printf("# Zero div check\n");
    printf("POPS LF@$op_tmp_1\n");
    printf("TYPE LF@$op_tmp_2 LF@$op_tmp_1\n");
    printf("JUMPIFEQ &zero_div_check_number_%d LF@$op_tmp_2 string@float\n",
            zero_div_check_cnt);
    printf("JUMPIFNEQ &zero_div_check_ok_%d LF@$op_tmp_1 int@0\n", zero_div_check_cnt);
    printf("JUMP &zero_div_check_not_ok_%d\n", zero_div_check_cnt);
    printf("LABEL &zero_div_check_number_%d\n", zero_div_check_cnt);
    printf("JUMPIFNEQ &zero_div_check_ok_%d LF@$op_tmp_1 float@0x0p+0\n", zero_div_check_cnt);
    printf("LABEL &zero_div_check_not_ok_%d\n", zero_div_check_cnt);
    printf("EXIT int@%u\n", EZERODIV);
    printf("LABEL &zero_div_check_ok_%d\n", zero_div_check_cnt);
    printf("PUSHS LF@$op_tmp_1\n");

    zero_div_check_cnt++;
}

void gen_conv_to_number_top()
{
    printf("# Convert top to number\n");
    printf("INT2FLOATS\n");
}

void gen_conv_to_number_subtop()
{
    printf("# Convert subtop to number\n");
    printf("POPS LF@$op_tmp_1\n");
    printf("INT2FLOATS\n");
    printf("PUSHS LF@$op_tmp_1\n");
}

void gen_write_identifier(identifier_t *id)
{
    gen_create_frame();
    printf("PUSHS LF@%s_%lu_%lu\n",
            id->name,
            id->line,
            id->character);
    printf("DEFVAR TF@%%params\n");
    printf("MOVE TF@%%params int@1\n");
    printf("CALL &write\n");
}

void gen_write_integer(int i)
{
    gen_create_frame();
    printf("PUSHS int@%d\n", i);
    printf("DEFVAR TF@%%params\n");
    printf("MOVE TF@%%params int@1\n");
    printf("CALL &write\n");
}

void gen_write_number(double n)
{
    gen_create_frame();
    printf("PUSHS float@%a\n", n);
    printf("DEFVAR TF@%%params\n");
    printf("MOVE TF@%%params int@1\n");
    printf("CALL &write\n");
}

void gen_write_string(char *s)
{
    gen_create_frame();
    printf("PUSHS string@%s\n", s);
    printf("DEFVAR TF@%%params\n");
    printf("MOVE TF@%%params int@1\n");
    printf("CALL &write\n");
}

// TODO this call is redundant, but for the sake of consistency
void gen_write_nil()
{
    gen_create_frame();
    printf("PUSHS nil@nil\n");
    printf("DEFVAR TF@%%params\n");
    printf("MOVE TF@%%params int@1\n");
    printf("CALL &write\n");
}
