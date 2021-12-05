/**
 * @file exprstack.h
 * Header of stack for expression syntactic analysis (bottom to top method)
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */

#ifndef _EXPRSTACK_H_
#define _EXPRSTACK_H_

#include "token.h"

/**
 * Available types of exprstack item
 */
enum exprstack_item_types {
    NON_TERM, STOP, TERM
};


/**
 * Type of the non-terminal
 *
 * @details
 * It tells what was reduced to this non-terminal
 * <ul>
 *      <li><code>N_VAL</code> - simple terminal (rule N --> T)</li>
 *      <li><code>N_EXPR</code> - expression (rule N --> N+N etc.)</li>
 *      <li><code>N_ERR</code> - rule cannot be applied</li>
 * </ul>
 */
enum non_term_types {
    N_VAL, N_EXPR, N_ERR
};

/**
 * Data of the non-terminal
 */
struct non_term_data {
    enum non_term_types type;
    token_t data;
};

/**
 * Item of expression stack
 *
 * @details
 * It can contain non-terminal (it's general in bottom-to-top analysis),
 * stop (\<) and terminal (stored in token). Items are linked from top
 * to bottom of the stack, so every item has a link to the previous one.
 */
struct exprstack_item {
    enum exprstack_item_types type;
    union {
        token_t term_data;
        struct non_term_data non_term_data;
    };
    struct exprstack_item *prev;
};

/**
 * Structure for storing metadata of expression stack
 */
struct exprstack {
    struct exprstack_item *top;
    struct exprstack_item *active;
};

/**
 * Stack for expression static analysis
 */
typedef struct exprstack exprstack_t;

/**
 * Special type for non-terminals for allowing to store required data
 */
typedef struct non_term_data non_term_t;

/**
 * Creates a new stack for expressions analysis
 *
 * @return Pointer to the new stack or NULL if error occurred
 */
exprstack_t *exprstack_create(void);

/**
 * Adds a new terminal onto the top of the expressions stack
 *
 * @param s Expressions stack to modify
 * @param term Terminal to push
 * @return 0 if success, negative value elsewhere
 * @pre s != NULL
 */
int exprstack_push_term(exprstack_t *s, token_t term);

/**
 * Adds a new non-terminal onto the top of the expressions stack
 *
 * @param s Expressions stack to modify
 * @param data Initial data for the non-terminal (in non-terminal format)
 * @return 0 if success, negative value elsewhere
 * @pre s != NULL
 */
int exprstack_push_non_term(exprstack_t *s, non_term_t data);

/**
 * Adds a new stop after the nearest terminal from the top of the stack
 *
 * @param s Expressions stack to modify
 * @return 0 if success, negative value elsewhere
 * @pre s != NULL
 */
int exprstack_add_stop_after_top_term(exprstack_t *s);

/**
 * Remove items from the top of the expressions stack ending on the stop
 *
 * @details
 * The stop item will be deleted, too
 *
 * @param s Expressions stack to modify
 * @pre s != NULL
 */
void exprstack_pop_to_stop(exprstack_t *s);

/**
 * Returns nearest terminal from the top of the expressions stack
 *
 * @param s Expressions stack
 * @return Pointer to the nearest terminal from the top, NULL if the stack contains no terminals
 * @pre s != NULL
 */
token_t *exprstack_top_term(exprstack_t *s);

/**
 * Returns nearest non-terminal from the top of the exprstack
 *
 * @param s Expressions stack
 * @return Pointer to data of the non-terminal from the top of the stack or NULL if the stack is out
 * of non-terminals
 * @pre s != NULL
 * @see exprstack_next_non_term()
 */
non_term_t *exprstack_top_non_term(exprstack_t *s);

/**
 * Returns next (2nd, 3rd, ... from the top) non-terminal
 *
 * @details
 * This function returns next (really it's the previous one, "next" means "the other one"
 * in this case) non-terminal of the selected one by exprstack_top_non_term() or this function.
 * When you need to start again from the top of the stack, just call exprstack_top_non_term()
 * which always returns non-terminal that is currently on the top and resets selection for
 * this function, so the 2nd call after *_top_non_term() always returns the 2nd non-terminal
 * from the top of the stack.
 *
 * @param s Expressions stack
 * @return Pointer to next available non-terminal's data or NULL if there is no other non-terminal in the stack
 * @pre s != NULL
 * @pre call exprstack_top_non_term() before
 * @see exprstack_top_non_term()
 */
non_term_t *exprstack_next_non_term(exprstack_t *s);

/**
 * Checks if the expressions stack has the rule on top
 *
 * @details
 * Rule is a string containing 'T' and 'N' characters only, where:
 * <ul>
 *      <li><code>T</code> is terminal and needs to be specified (3rd and next parameters)</li>
 *      <li><code>N</code> is non-terminal and doesn't need specification</li>
 * </ul>
 * Terminals are supplied as parameters after the rule parameter. Their order depends on the
 * order in rule. For example: rule="T<sub>1</sub>NT<sub>2</sub>" --> params: rule, T<sub>1</sub>,
 * T<sub>2</sub>
 *
 * @param s Expressions stack
 * @param rule Rule to check
 * @param ... Terminals' values
 * @return Is the rule on top of the stack?
 * @pre s != NULL
 * @pre rule != NULL
 */
bool exprstack_check_top(exprstack_t *s, char *rule, ...);

/**
 * Checks if the expressions stack is correctly "empty" (see details)
 *
 * @details
 * Empty expressions stack has just one END data at the bottom,
 * and at most one non-terminal at the top ($) or ($N)
 *
 * @param s Expressions stack
 * @return Is it correctly empty (see definition in details)?
 * @pre s != NULL
 */
bool exprstack_is_correctly_empty(exprstack_t *s);

/**
 * Destroys an existing expressions stack
 *
 * @param s Pointer to expressions stack to be destroyed
 * @pre s != NULL
 */
void exprstack_destroy(exprstack_t *s);

#endif //_EXPRSTACK_H_
