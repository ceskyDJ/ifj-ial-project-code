/**
 * @file expr_parser.c
 * Parser for expressions (syntactic analyser using bottom-to-top method)
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */

#include "expr_parser.h"
#include "scanner.h"
#include "exprstack.h"
#include "token.h"
#include "exit_codes.h"
#include "logger.h"
#include "symtable.h"
#include "symstack.h"
#include "generator.h"
#include <stdio.h>
#include <string.h>

// START testing - for testing purposes only, REMOVE for production!
char *token_types[] = {
        "IDENTIFIER", "KEYWORD", "INTEGER", "NUMBER", "STRING", "BOOL", "NIL", "ASSIGNMENT", "ADDITION", "SUBTRACTION",
        "MULTIPLICATION", "DIVISION", "INT_DIVISION", "CONCAT", "STRLEN", "LT", "GT", "LEQ", "GEQ", "EQ",
        "NEQ", "TYPE_SPEC", "COMMA", "LEFT_PAR", "RIGHT_PAR","END"
};

char *term_types[] = {
        "id", "nil", "integer", "number", "string", "bool", "nil", "ERR", "+", "-",
        "*", "/", "//", "..", "#", "<", ">", "<=", ">=", "==",
        "~=", "ERR", "ERR", "(", ")","$"
};

char *keywords[] = {
        "DO", "ELSE", "END", "FUNCTION", "GLOBAL", "IF", "INTEGER", "LOCAL", "NIL", "NUMBER", "REQUIRE",
        "RETURN", "STRING", "THEN", "WHILE"
};

static void print_exprstack(exprstack_t *s)
{
    struct exprstack_item *item = s->top;
    char status[513];
    int status_len = 0;
    char *tmp_string;
    char tmp_char;

    memset(status, '\0', 512);

    if (!item) {
        LOG_DEBUG_M("Exprstack status: completely empty");
        return;
    }

    while (item) {
        if (item->type == NON_TERM)
            status[status_len++] = 'N';
        else if (item->type == STOP)
            status[status_len++] = '|';
        else if (item->type == TERM) {
            tmp_string = term_types[item->term_data.type];
            for (int i = 0; i < (int) strlen(tmp_string); i++) {
                status[status_len + strlen(tmp_string) - i - 1] = tmp_string[i];
            }
            status_len += (int) strlen(tmp_string);
        }

        status[status_len++] = ' ';

        item = item->prev;
    }

    for (int i = 0; i < status_len / 2; i++) {
        tmp_char = status[i];
        status[i] = status[status_len - i - 1];
        status[status_len - i - 1] = tmp_char;
    }

    LOG_DEBUG("Exprstack status: %s", status);
}
// END testing

static non_term_t strlen_rule(exprstack_t *s);
static non_term_t mul_rule(exprstack_t *s);
static non_term_t div_rule(exprstack_t *s);
static non_term_t int_div_rule(exprstack_t *s);
static non_term_t add_rule(exprstack_t *s);
static non_term_t sub_rule(exprstack_t *s);
static non_term_t concat_rule(exprstack_t *s);
static non_term_t lt_rule(exprstack_t *s);
static non_term_t leq_rule(exprstack_t *s);
static non_term_t gt_rule(exprstack_t *s);
static non_term_t geq_rule(exprstack_t *s);
static non_term_t eq_rule(exprstack_t *s);
static non_term_t neq_rule(exprstack_t *s);
static non_term_t par_rule(exprstack_t *s);
static non_term_t term_rule(exprstack_t *s);

/**
 * Array of available rules
 */
rule_fun_t rules[RULES_NUM] = {
        strlen_rule, mul_rule, div_rule, int_div_rule, add_rule, sub_rule, concat_rule, lt_rule, leq_rule, gt_rule,
        geq_rule, eq_rule, neq_rule, par_rule, term_rule
};

/**
 * Precedence table for controlling bottom-to-top syntactic analysis
 */
enum expr_parser_operations precedence_table[17][17] = {
        {R_NON, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_NON, R_NON, R_NON, R_NON, R_NON, R_NON, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_NON, R_NON, R_NON, R_NON, R_NON, R_NON, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_NON, R_NON, R_NON, R_NON, R_NON, R_NON, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_NON, R_NON, R_NON, R_NON, R_NON, R_NON, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_NON, R_NON, R_NON, R_NON, R_NON, R_NON, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_NON, R_NON, R_NON, R_NON, R_NON, R_NON, R_SHT, R_RDC, R_SHT, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_PSH, R_SHT, R_NON},
        {R_NON, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_NON, R_RDC, R_NON, R_RDC},
        {R_NON, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_RDC, R_NON, R_RDC, R_NON, R_RDC},
        {R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_SHT, R_NON, R_SHT, R_NON},
};

/**
 * Header (both row and column) of precedence table
 */
enum token_type precedence_tab_header[PREC_TAB_HEADER_SIZE] = {
        STRLEN, MULTIPLICATION, DIVISION, INT_DIVISION, ADDITION, SUBTRACTION, CONCAT, LT, LEQ, GT, GEQ, EQ, NEQ,
        LEFT_PAR, RIGHT_PAR /* , TERM */, END
};

static int get_table_header_index(token_t token)
{
    enum token_type type = token.type;
    int index = -1;

    if (type == INTEGER || type == NUMBER || type == STRING || type == IDENTIFIER
        || (type == KEYWORD && *token.keyword == KW_NIL))
        return PREC_TAB_TERM_POS;

    for (int i = 0; i < PREC_TAB_HEADER_SIZE; i++) {
        if (precedence_tab_header[i] == token.type) {
            index = i;
            break;
        }
    }

    if (index != -1 && type != END)
        return index;
    if (index != -1 && type == END)
        // One position is reserved for term, that cannot be there explicitly
        return index + 1;
    else
        return -1;
}

static enum expr_parser_operations *table_lookup(token_t on_stack, token_t input)
{
    int row = get_table_header_index(on_stack);
    int col = get_table_header_index(input);
    token_t term_token = {.type = IDENTIFIER};

    // We must end with expression processing, when there is terminal on the stack and identifier on the input,
    // because assignment follows, and current token actually isn't for the expression parser
    if (row == get_table_header_index(term_token) && input.type == IDENTIFIER)
        return NULL;

    if (row == -1 || col == -1)
        return NULL;
    else
        return &precedence_table[row][col];
}

static bool try_implicit_conversion(non_term_t *first_op, non_term_t *second_op)
{
    if (first_op->data.type == INTEGER && second_op->data.type == NUMBER) {
        gen_conv_to_number_subtop();
        first_op->data.type = NUMBER;
        LOG_DEBUG_M("Applying implicit conversion from integer to number for the previous operand...");

        return true;
    } else if (first_op->data.type == NUMBER && second_op->data.type == INTEGER) {
        gen_conv_to_number_top();
        second_op->data.type = NUMBER;
        LOG_DEBUG_M("Applying implicit conversion from integer to number for the current operand...");

        return true;
    }

    return false;
}

static void check_same_types(non_term_t *first_op, non_term_t *second_op, bool nil_allowed)
{
    // Types must be the same or compatible
    if (first_op->data.type == second_op->data.type)
        return;

    // One of the operand could be of type NIL, if allowed
    if (nil_allowed && (first_op->data.type == NIL || second_op->data.type == NIL))
        return;

    // If we have two numeric types, implicit conversion resolves this
    if (try_implicit_conversion(first_op, second_op))
        return;

    // Implicit conversion cannot be done --> incompatible types
    LOG_ERROR("Bad type of operands in expression: %s, %s", term_types[first_op->data.type],
            term_types[second_op->data.type]);
    exit(EEXPTYPE);
}

static bool is_valid_variable(context_t *ctx, token_t *token)
{
    symtable_t *symtable;
    identifier_t *tmp_id;

    // Go through all tables of symbols and try to find checked identifier with type (declared variable)
    symstack_most_local(ctx->symstack);
    while (symstack_is_active(ctx->symstack)) {
        symtable = symstack_get(ctx->symstack);
        tmp_id = symtable_find(symtable, token->identifier->name);

        if (tmp_id && tmp_id->type == VARIABLE) {
            // It's a valid variable, so we are done here
            token->identifier = tmp_id;
            return true;
        }

        symstack_next(ctx->symstack);
    }

    return false;
}

static non_term_t strlen_rule(exprstack_t *s)
{
    token_t strlen_term = {.type = STRLEN};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operand
    non_term_t *operand;

    if (!exprstack_check_top(s, "TN", strlen_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> #N");

    // Semantic checks
    operand = exprstack_top_non_term(s);

    CHECK_TYPE(operand, STRING);
    gen_nil_check_top();

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = INTEGER;

    // Semantic actions for generating code
    gen_operation(strlen_term.type);

    return result_non_term;
}

static non_term_t mul_rule(exprstack_t *s)
{
    token_t mul_term = {.type = MULTIPLICATION};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", mul_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N*N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    CHECK_NUMERIC(first_op);
    gen_nil_check_subtop();
    CHECK_NUMERIC(second_op);
    gen_nil_check_top();

    // Implicit conversion integer --> number
    try_implicit_conversion(first_op, second_op);

    // Set result type of expression
    result_non_term.type = N_EXPR;
    if (first_op->data.type == INTEGER)
        result_non_term.data.type = INTEGER;
    else
        result_non_term.data.type = NUMBER;

    // Semantic actions for generating code
    gen_operation(mul_term.type);

    return result_non_term;
}

static non_term_t div_rule(exprstack_t *s)
{
    token_t div_term = {.type = DIVISION};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", div_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N/N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    CHECK_NUMERIC(first_op);
    gen_nil_check_subtop();
    CHECK_NUMERIC(second_op);
    gen_nil_check_top();
    if (second_op->type == N_VAL) {
        if ((second_op->data.type == INTEGER && second_op->data.integer == 0)
            || (second_op->data.type == NUMBER && second_op->data.number == 0.0)) {
            LOG_ERROR_M("Division by zero literal in expression");
            exit(EZERODIV);
        }
    }
    gen_zero_div_check();

    // Implicit conversion integer --> number
    try_implicit_conversion(first_op, second_op);

    // Both operands must be of type number
    if (first_op->data.type == INTEGER && second_op->data.type == INTEGER) {
        gen_conv_to_number_subtop();
        gen_conv_to_number_top();
    }

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = NUMBER;

    // Semantic actions for generating code
    gen_operation(div_term.type);

    return result_non_term;
}

static non_term_t int_div_rule(exprstack_t *s)
{
    token_t int_div_term = {.type = INT_DIVISION};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", int_div_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N//N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    CHECK_TYPE(first_op, INTEGER);
    CHECK_TYPE(second_op, INTEGER);
    gen_nil_check_subtop();
    if (second_op->data.integer == 0) {
        LOG_ERROR_M("Division by zero literal in expression");
        exit(EZERODIV);
    }
    gen_nil_check_top();
    gen_zero_div_check();

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = INTEGER;

    // Semantic actions for generating code
    gen_operation(int_div_term.type);

    return result_non_term;
}

static non_term_t add_rule(exprstack_t *s)
{
    token_t add_term = {.type = ADDITION};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", add_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N+N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    CHECK_NUMERIC(first_op);
    gen_nil_check_subtop();
    CHECK_NUMERIC(second_op);
    gen_nil_check_top();

    // Implicit conversion integer --> number
    try_implicit_conversion(first_op, second_op);

    // Set result type of expression
    result_non_term.type = N_EXPR;
    if (first_op->data.type == INTEGER)
        result_non_term.data.type = INTEGER;
    else
        result_non_term.data.type = NUMBER;

    // Semantic actions for generating code
    gen_operation(add_term.type);

    return result_non_term;
}

static non_term_t sub_rule(exprstack_t *s)
{
    token_t sub_term = {.type = SUBTRACTION};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", sub_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N-N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    CHECK_NUMERIC(first_op);
    gen_nil_check_subtop();
    CHECK_NUMERIC(second_op);
    gen_nil_check_top();

    // Implicit conversion integer --> number
    try_implicit_conversion(first_op, second_op);

    // Set result type of expression
    result_non_term.type = N_EXPR;
    if (first_op->data.type == INTEGER)
        result_non_term.data.type = INTEGER;
    else
        result_non_term.data.type = NUMBER;

    // Semantic actions for generating code
    gen_operation(sub_term.type);

    return result_non_term;
}

static non_term_t concat_rule(exprstack_t *s)
{
    token_t concat_term = {.type = CONCAT};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", concat_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N..N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    CHECK_TYPE(first_op, STRING);
    gen_nil_check_subtop();
    CHECK_TYPE(second_op, STRING);
    gen_nil_check_top();

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = STRING;

    // Semantic actions for generating code
    gen_operation(concat_term.type);

    return result_non_term;
}

static non_term_t lt_rule(exprstack_t *s)
{
    token_t lt_term = {.type = LT};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", lt_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N<N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    check_same_types(first_op, second_op, false);
    gen_nil_check_subtop();
    gen_nil_check_top();

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = BOOL;

    // Semantic actions for generating code
    gen_operation(lt_term.type);

    return result_non_term;
}

static non_term_t leq_rule(exprstack_t *s)
{
    token_t leq_term = {.type = LEQ};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", leq_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N<=N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    check_same_types(first_op, second_op, false);
    gen_nil_check_subtop();
    gen_nil_check_top();

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = BOOL;

    // Semantic actions for generating code
    gen_operation(leq_term.type);

    return result_non_term;
}

static non_term_t gt_rule(exprstack_t *s)
{
    token_t gt_term = {.type = GT};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", gt_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N>N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    check_same_types(first_op, second_op, false);
    gen_nil_check_subtop();
    gen_nil_check_top();

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = BOOL;

    // Semantic actions for generating code
    gen_operation(gt_term.type);

    return result_non_term;
}

static non_term_t geq_rule(exprstack_t *s)
{
    token_t geq_term = {.type = GEQ};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", geq_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N>=N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    check_same_types(first_op, second_op, false);
    gen_nil_check_subtop();
    gen_nil_check_top();

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = BOOL;

    // Semantic actions for generating code
    gen_operation(geq_term.type);

    return result_non_term;
}

static non_term_t eq_rule(exprstack_t *s)
{
    token_t eq_term = {.type = EQ};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", eq_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N==N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    check_same_types(first_op, second_op, true);

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = BOOL;

    // Semantic actions for generating code
    gen_operation(eq_term.type);

    return result_non_term;
}

static non_term_t neq_rule(exprstack_t *s)
{
    token_t neq_term = {.type = NEQ};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *first_op;
    non_term_t *second_op;

    if (!exprstack_check_top(s, "NTN", neq_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> N~=N");

    // Semantic checks
    second_op = exprstack_top_non_term(s);
    first_op = exprstack_next_non_term(s);

    check_same_types(first_op, second_op, true);

    // Set result type of expression
    result_non_term.type = N_EXPR;
    result_non_term.data.type = BOOL;

    // Semantic actions for generating code
    gen_operation(neq_term.type);

    return result_non_term;
}

static non_term_t par_rule(exprstack_t *s)
{
    token_t left_par_term = {.type = LEFT_PAR};
    token_t right_par_term = {.type = RIGHT_PAR};
    non_term_t result_non_term = {.type = N_ERR};
    // Data of operands
    non_term_t *operand;

    if (!exprstack_check_top(s, "TNT", left_par_term, right_par_term))
        return result_non_term;

    LOG_DEBUG_M("Used rule: N --> (N)");

    operand = exprstack_top_non_term(s);

    // Set result type of expression
    // This is rule for parentheses, there is no required semantic checks or actions
    // Output is just copied from contained non-terminal
    result_non_term = *operand;

    return result_non_term;
}

static non_term_t term_rule(exprstack_t *s)
{
    token_t int_term = {.type = INTEGER};
    token_t num_term = {.type = NUMBER};
    token_t str_term = {.type = STRING};
    token_t id_term = {.type = IDENTIFIER};
    keyword_t nil_kw = KW_NIL;
    token_t nil_term = {.type = KEYWORD, .keyword = &nil_kw};
    non_term_t result_non_term = {.type = N_ERR};
    identifier_t identifier;
    token_t *top_term;

    // Get terminal's token from the top of the exprstack
    top_term = exprstack_top_term(s);
    if (!top_term)
        // There is no terminal at the top, so no term rule will pass
        return result_non_term;

    // Literals and identifiers inherits data from token
    result_non_term.data = *top_term;

    if (exprstack_check_top(s, "T", int_term)) {
        LOG_DEBUG_M("Used rule: N <-- T (T = integer)");
        result_non_term.data.type = INTEGER;
    } else if (exprstack_check_top(s, "T", num_term)) {
        LOG_DEBUG_M("Used rule: N <-- T (T = number)");
        result_non_term.data.type = NUMBER;
    } else if (exprstack_check_top(s, "T", str_term)) {
        LOG_DEBUG_M("Used rule: N <-- T (T = string)");
        result_non_term.data.type = STRING;
    } else if (exprstack_check_top(s, "T", id_term)) {
        LOG_DEBUG_M("Used rule: N <-- T (T = identifier)");

        // Simplify type of output token
        // We won't need all information about this term in other rules
        identifier = *top_term->identifier;
        switch (identifier.var.type) {
            case VAR_INTEGER:
                result_non_term.data.type = INTEGER;
                break;
            case VAR_NUMBER:
                result_non_term.data.type = NUMBER;
                break;
            case VAR_STRING:
                result_non_term.data.type = STRING;
                break;
            default:
                result_non_term.type = N_ERR;
                result_non_term.data.type = END;
                break;
        }
    } else if (exprstack_check_top(s, "T", nil_term)) {
        LOG_DEBUG_M("Used rule: N <-- T (T = nil)");
        result_non_term.data.type = NIL;
    } else
        return result_non_term;

    // We always reduce simple term
    result_non_term.type = N_VAL;

    // Semantic actions for generating code
    gen_push_term(top_term);

    return result_non_term;
}

enum variable_type expr_parser_start(context_t *context)
{
    exprstack_t *exprstack = exprstack_create();
    token_t end_token = {.type = END};
    token_t input_token;
    token_t *stack_token;
    enum expr_parser_operations *operation;
    rule_fun_t rule;
    non_term_t rule_exec_result = {.type = N_ERR};
    bool done = false;

    if (!exprstack)
        exit(EINTERNAL);

    LOG_DEBUG_M("Starting expression parser...");

    exprstack_push_term(exprstack, end_token);
    print_exprstack(exprstack);

    input_token = get_next_token(context);
    LOG_DEBUG("Loaded new token of type: %s", token_types[input_token.type]);
    while (!done || !exprstack_is_correctly_empty(exprstack)) {
        stack_token = exprstack_top_term(exprstack);
        LOG_DEBUG("Current term on the top of exprstack: %s", token_types[stack_token->type]);
        operation = table_lookup(*stack_token, input_token);
        if (!operation) {
            // Return token to the scanner (it will be needed in the main parser)
            unget_token(input_token);
            LOG_DEBUG_M("Putting back read token to the scanner...");

            // Next tokens will be only "fake" END ones
            LOG_DEBUG_M("Unknown token detected, ending with parsing loaded expression...");
            done = true;

            // Set "fake" END token, which is needed for correct finishing of expression
            // syntactic analysis
            input_token = end_token;
            LOG_DEBUG_M("Loaded \"fake\" END token");
            continue;
        }

        switch (*operation) {
            case R_PSH:
                LOG_DEBUG_M("Selected operation (from precedence table): PUSH");
                exprstack_push_term(exprstack, input_token);

                print_exprstack(exprstack);
                break;
            case R_SHT:
                if (input_token.type == IDENTIFIER && !is_valid_variable(context, &input_token)) {
                    LOG_ERROR_M("Operand isn't variable");
                    exit(ESEM_OTHER); // FIXME: maybe another type of error
                }

                LOG_DEBUG_M("Selected operation (from precedence table): SHIFT");
                exprstack_add_stop_after_top_term(exprstack);
                exprstack_push_term(exprstack, input_token);

                print_exprstack(exprstack);
                break;
            case R_RDC:
                LOG_DEBUG_M("Selected operation (from precedence table): REDUCE");
                for (int i = 0; i < RULES_NUM; i++) {
                    rule = rules[i];

                    rule_exec_result = rule(exprstack);
                    if (rule_exec_result.type != N_ERR) {
                        break;
                    }
                }

                if (rule_exec_result.type != N_ERR) {
                    exprstack_pop_to_stop(exprstack);
                    exprstack_push_non_term(exprstack, rule_exec_result);

                    print_exprstack(exprstack);
                } else {
                    LOG_ERROR_M("There is no operation to apply");
                    exit(ESYNTAX);
                }

                // Do not do anything else (especially read next token),
                // it's done only while shifting and pushing (first 2 operations)
                LOG_DEBUG("Get new token has been skipped. Using old token: %s", token_types[input_token.type]);
                continue;
            case R_NON:
                LOG_DEBUG_M("Selected operation (from precedence table): NONE");
                LOG_ERROR_M("Bad token arrived in the expression static analysis");
                exit(ESYNTAX);
        }

        // Move to the next input_token
        if (!done) {
            input_token = get_next_token(context);
            LOG_DEBUG("Loaded new token of type: %s", token_types[input_token.type]);
        } else {
            // At the end we need to generate "fake" END tokens,
            // because expression on input has ended, and we
            // need to simulate the end of the whole input
            input_token = end_token;
            LOG_DEBUG_M("Loaded \"fake\" END token");
        }
    }

    LOG_DEBUG_M("Switching back to top-to-bottom syntactic analysis...");

    // Clean up jobs
    exprstack_destroy(exprstack);

    // Return type of the expression result
    if (rule_exec_result.data.type == INTEGER)
        return VAR_INTEGER;
    else if (rule_exec_result.data.type == NUMBER)
        return VAR_NUMBER;
    else if (rule_exec_result.data.type == BOOL)
        return VAR_BOOL;
    else
        return VAR_STRING;
}
