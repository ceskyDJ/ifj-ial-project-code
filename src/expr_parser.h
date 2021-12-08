/**
 * @file expr_parser.h
 * Header of expressions parser (syntactic analyser using bottom-to-top method)
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */

#ifndef _EXPR_PARSER_H_
#define _EXPR_PARSER_H_

#include "token.h"
#include "context.h"
#include "exprstack.h"
#include "scanner.h"

/**
 * Size precedence table header (number of column/rows in there)
 */
#define PREC_TAB_HEADER_SIZE 17
/**
 * Position of TERM item in precedence table header
 */
#define PREC_TAB_TERM_POS 15
/**
 * Number of rules in this static analysis
 */
#define RULES_NUM 15

/**
 * Checks if the operand is numeric
 */
#define CHECK_NUMERIC(operand) do { \
    if (operand->data.type != INTEGER && operand->data.type != NUMBER) { \
        LOG_ERROR("Bad type of operand in expression: %s", term_types[operand->data.type]); \
        exit(EEXPTYPE); \
    } \
} while(0)
/**
 * Checks if the operand has the correct type
 */
#define CHECK_TYPE(operand, right_type) do { \
    if (operand->data.type != right_type) { \
        LOG_ERROR("Bad type of operand in expression: %s", term_types[operand->data.type]); \
        exit(EEXPTYPE); \
    } \
} while(0)

/**
 * Operation for bottom-to-top syntactic analysis
 *
 * @details
 * <ul>
 *      <li><code>R_SHT</code> (\<) - shift operation and input into the stack</li>
 *      <li><code>R_PSH</code> (\=) - special case of shift (only push input to the stack)</li>
 *      <li><code>R_RDC</code> (\>) - reduce items on the stack with non-terminal</li>
 *      <li><code>R_NON</code> (empty) - forbidden combination</li>
 * </ul>
 */
enum expr_parser_operations {
    R_SHT, R_PSH, R_RDC, R_NON
};

/**
 * Function for applying a rule when it's on exprstack
 */
typedef non_term_t (*rule_fun_t)(exprstack_t *);

/**
 * Checks if identifier in the provided token is a variable
 *
 * @details
 * Valid variable is of type VARIABLE, which means it's ready to use.
 * Token is passed by pointer and will be changed! The identifier in
 * it will be replaced with its valid version. This is applied when
 * the most local version isn't valid. This function finds nearest
 * valid version (from the nearest table of symbols).
 *
 * @param ctx Pointer to context with dependencies
 * @param token Pointer to token with the identifier
 * @return Is it a valid variable?
 */
bool is_valid_variable(context_t *ctx, token_t *token);

/**
 * Runs parsing expressions until valid input will arrive
 *
 * @details
 * Valid input is set of tokens allowed to be in expressions.
 *
 * @param context Context with dependencies
 * @return Final type of the processed expression (of its result, resp.)
 */
enum variable_type expr_parser_start(context_t *context);

#endif //_EXPR_PARSER_H_
