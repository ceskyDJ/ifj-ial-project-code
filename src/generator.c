/*
 * @file generator.c
 * Generator implementation.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 * @author Michal Šmahel (xsmahe01)
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

void gen_reads(void)
{
    printf("# reads(): string\n");
    printf("LABEL &reads\n");
    printf("PUSHFRAME\n");

    printf("DEFVAR LF@%%retval_1\n");

    printf("READ LF@%%retval_1 string\n");

    printf("POPFRAME\n");
    printf("RETURN\n");
}

void gen_readi(void)
{
    printf("# readi(): integer\n");
    printf("LABEL &readi\n");
    printf("PUSHFRAME\n");

    printf("DEFVAR LF@%%retval_1\n");

    printf("READ LF@%%retval_1 int\n");

    printf("POPFRAME\n");
    printf("RETURN\n");
}

void gen_readn(void)
{
    printf("# readn(): number\n");
    printf("LABEL &readn\n");
    printf("PUSHFRAME\n");

    printf("DEFVAR LF@%%retval_1\n");

    printf("READ LF@%%retval_1 float\n");

    printf("POPFRAME\n");
    printf("RETURN\n");
}

void gen_write(void)
{
    printf("# write(any)\n");
    printf("LABEL &write\n");
    printf("PUSHFRAME\n");

    printf("DEFVAR LF@$type\n");

    printf("TYPE LF@$type LF@%%1\n");
    printf("JUMPIFEQ &if_nil_%u LF@$type string@nil\n", if_cnt);
    printf("WRITE LF@%%1\n");
    printf("JUMP &if_end_%u\n", if_cnt);
    printf("LABEL &if_nil_%u\n", if_cnt);
    printf("WRITE string@nil\n");
    printf("LABEL &if_end_%u\n", if_cnt);
    if_cnt++;

    printf("POPFRAME\n");
    printf("RETURN\n");
}

void gen_tointeger(void)
{
    printf("# tointeger(number): integer\n");
    printf("LABEL &tointeger\n");
    printf("PUSHFRAME\n");

    printf("DEFVAR LF@%%retval_1\n");
    printf("MOVE LF@%%retval_1 nil@nil\n");
    printf("DEFVAR LF@$type\n");

    printf("TYPE LF@$type LF@%%1\n");
    printf("JUMPIFEQ &if_nil_%u LF@$type string@nil\n", if_cnt);
    printf("FLOAT2INT LF@%%retval_1 LF@%%1\n");
    printf("LABEL &if_nil_%u\n", if_cnt);
    if_cnt++;

    printf("POPFRAME\n");
    printf("RETURN\n");
}

void gen_substr(void)
{
    printf("# substr(string, number, number): string\n");
    printf("LABEL &substr\n");
    printf("PUSHFRAME\n");

    printf("DEFVAR LF@%%retval_1\n");
    printf("DEFVAR LF@$tmp_c\n");
    printf("DEFVAR LF@$counter\n");
    printf("DEFVAR LF@$limit\n");
    printf("DEFVAR LF@$type\n");
    printf("DEFVAR LF@$lim_cond\n");
    printf("DEFVAR LF@$lim_cond_2\n");
    printf("DEFVAR LF@$len\n");

    printf("# 1st param != nil\n");
    printf("TYPE LF@$type LF@%%1\n");
    printf("JUMPIFEQ &if_nil_%u LF@$type string@nil\n", if_cnt);
    printf("# 2nd param != nil\n");
    printf("TYPE LF@$type LF@%%2\n");
    printf("JUMPIFEQ &if_nil_%u LF@$type string@nil\n", if_cnt);
    printf("# 3rd param != nil\n");
    printf("TYPE LF@$type LF@%%3\n");
    printf("JUMPIFEQ &if_nil_%u LF@$type string@nil\n", if_cnt);
    printf("JUMP &if_end_%u\n", if_cnt);
    printf("# Bad param error\n");
    printf("LABEL &if_nil_%u\n", if_cnt);
    printf("EXIT int@8\n");
    printf("LABEL &if_end_%u\n", if_cnt);
    if_cnt++;

    printf("FLOAT2INT LF@$counter LF@%%2\n");
    printf("FLOAT2INT LF@$limit LF@%%3\n");
    printf("# Limits checks\n");
    printf("STRLEN LF@$len LF@%%1\n");
    printf("LT LF@$lim_cond LF@$counter int@1\n");
    printf("GT LF@$lim_cond_2 LF@$counter LF@$limit\n");
    printf("OR LF@$lim_cond LF@$lim_cond LF@$lim_cond_2\n");
    printf("GT LF@$lim_cond_2 LF@$limit LF@$len\n");
    printf("OR LF@$lim_cond LF@$lim_cond LF@$lim_cond_2\n");
    printf("JUMPIFEQ &if_lim_bad_%u LF@$lim_cond bool@true\n", if_cnt);
    printf("SUB LF@$counter LF@$counter int@1\n");

    printf("# Create result string\n");
    printf("MOVE LF@%%retval_1 string@\n");
    printf("LABEL &while_%u\n", while_cnt);
    printf("JUMPIFEQ &while_end_%u LF@$counter LF@$limit\n", while_cnt);
    printf("GETCHAR LF@$tmp_c LF@%%1 LF@$counter\n");
    printf("CONCAT LF@%%retval_1 LF@%%retval_1 LF@$tmp_c\n");
    printf("ADD LF@$counter LF@$counter int@1\n");
    printf("JUMP &while_%u\n", while_cnt);
    printf("LABEL &while_end_%u\n", while_cnt);
    while_cnt++;

    printf("JUMP &if_lim_end_%u\n", if_cnt);
    printf("LABEL &if_lim_bad_%u\n", if_cnt);
    printf("MOVE LF@%%retval_1 string@\n");
    printf("LABEL &if_lim_end_%u\n", if_cnt);
    if_cnt++;

    printf("POPFRAME\n");
    printf("RETURN\n");
}

void gen_ord(void)
{
    printf("# ord(string, integer): integer\n");
    printf("LABEL &ord\n");
    printf("PUSHFRAME\n");

    printf("DEFVAR LF@$index\n");
    printf("DEFVAR LF@%%retval_1\n");
    printf("DEFVAR LF@$lim_cond\n");
    printf("DEFVAR LF@$lim_cond_2\n");
    printf("DEFVAR LF@$len\n");

    printf("MOVE LF@%%retval_1 nil@nil\n");
    printf("DEFVAR LF@$type\n");
    printf("TYPE LF@$type LF@%%1\n");
    printf("JUMPIFEQ &if_nil_%u LF@$type string@nil\n", if_cnt);
    printf("TYPE LF@$type LF@%%2\n");
    printf("JUMPIFEQ &if_nil_%u LF@$type string@nil\n", if_cnt);
    printf("STRLEN LF@$len LF@%%1\n");
    printf("LT LF@$lim_cond LF@%%2 int@1\n");
    printf("GT LF@$lim_cond_2 LF@%%2 LF@$len\n");
    printf("OR LF@$lim_cond LF@$lim_cond LF@$lim_cond_2\n");
    printf("JUMPIFEQ &if_nil_%u LF@$lim_cond bool@true\n", if_cnt);
    printf("SUB LF@$index LF@%%2 int@1\n");
    printf("STRI2INT LF@%%retval_1 LF@%%1 LF@$index\n");
    printf("LABEL &if_nil_%u\n", if_cnt);
    if_cnt++;

    printf("POPFRAME\n");
    printf("RETURN\n");
}

void gen_chr(void)
{
    printf("# chr(integer): string\n");
    printf("LABEL &chr\n");
    printf("PUSHFRAME\n");

    printf("DEFVAR LF@%%retval_1\n");
    printf("DEFVAR LF@$lim_cond\n");
    printf("DEFVAR LF@$lim_cond_2\n");
    printf("DEFVAR LF@$type\n");

    printf("MOVE LF@%%retval_1 nil@nil\n");
    printf("TYPE LF@$type LF@%%1\n");
    printf("JUMPIFNEQ &if_nil_%u LF@$type string@nil\n", if_cnt);
    printf("EXIT int@8\n");
    printf("LABEL &if_nil_%u\n", if_cnt);
    if_cnt++;

    printf("LT LF@$lim_cond LF@%%1 int@0\n");
    printf("GT LF@$lim_cond_2 LF@%%1 int@255\n");
    printf("OR LF@$lim_cond LF@$lim_cond LF@$lim_cond_2\n");
    printf("JUMPIFEQ &if_lim_%u LF@$lim_cond bool@true\n", if_cnt);
    printf("INT2CHAR LF@%%retval_1 LF@%%1\n");
    printf("LABEL &if_lim_%u\n", if_cnt);
    if_cnt++;

    printf("POPFRAME\n");
    printf("RETURN\n");
}

void gen_builtins(void)
{
    // We need to separate code of built-in functions from the code for normal executing
    printf("JUMP &builtins_skip\n\n");

    // I/O operations
    gen_reads();
    printf("\n");
    gen_readi();
    printf("\n");
    gen_readn();
    printf("\n");
    gen_write();
    printf("\n");

    // Conversions
    gen_tointeger();
    printf("\n");

    // String operations
    gen_substr();
    printf("\n");
    gen_ord();
    printf("\n");
    gen_chr();
    printf("\n");

    printf("LABEL &builtins_skip\n");
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

void gen_call_param(token_t *token, bool conv_to_number)
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

    if (conv_to_number)
        printf("INT2FLOAT TF@%%%u TF@%%%u\n",
                call_param_cnt,
                call_param_cnt);

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
    printf("DEFVAR TF@%%1\n");
    printf("MOVE TF@%%1 LF@%s_%lu_%lu\n",
            id->name,
            id->line,
            id->character);
    printf("CALL &write\n");
}

void gen_write_integer(int i)
{
    printf("WRITE int@%d\n", i);
}

void gen_write_number(double n)
{
    printf("WRITE float@%a\n", n);
}

void gen_write_string(char *s)
{
    printf("WRITE string@%s\n", s);
}

void gen_write_nil()
{
    printf("WRITE string@nil\n");
}
