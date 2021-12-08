/**
 * @file exprstack.c
 * Stack for syntactic analysis using bottom to top method for processing expressions
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */

#include "exprstack.h"
#include "logger.h"
#include "exit_codes.h"
#include <errno.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

static struct exprstack_item *create_item(exprstack_t *s)
{
    assert(s);

    struct exprstack_item *new_item = malloc(sizeof(struct exprstack_item));
    if (!new_item)
        return NULL;

    new_item->prev = s->top;
    s->top = new_item;

    return new_item;
}

static bool token_equal(token_t first, token_t second)
{
    if (first.type != second.type)
        return false;

    if (first.type == KEYWORD && *first.keyword != *second.keyword)
        return false;

    return true;
}

exprstack_t *exprstack_create(void)
{
    exprstack_t *s = malloc(sizeof(exprstack_t));
    if (!s)
        return NULL;

    s->top = NULL;
    s->active = NULL;

    return s;
}

int exprstack_push_term(exprstack_t *s, token_t term)
{
    assert(s);

    struct exprstack_item *new_item = create_item(s);
    if (!new_item)
        return -ENOMEM;

    new_item->type = TERM;
    new_item->term_data = term;

    return 0;
}

int exprstack_push_non_term(exprstack_t *s, non_term_t data)
{
    assert(s);

    struct exprstack_item *new_item = create_item(s);
    if (!new_item)
        return -ENOMEM;

    new_item->type = NON_TERM;
    new_item->non_term_data = data;

    return 0;
}

int exprstack_add_stop_after_top_term(exprstack_t *s)
{
    assert(s);

    struct exprstack_item *term_item = s->top;
    struct exprstack_item *next_item = NULL;
    struct exprstack_item *new_item;

    // Stack is empty
    if (!term_item)
        return -1;

    while (term_item->type != TERM) {
        next_item = term_item;
        term_item = term_item->prev;
    }

    // Create a new term_item
    new_item = malloc(sizeof(struct exprstack_item));
    if (!new_item)
        return -ENOMEM;

    // Setup new term_item
    new_item->type = STOP;
    new_item->prev = term_item;

    // Add a new term_item into stack
    if (!next_item)
        s->top = new_item;
    else
        next_item->prev = new_item;

    return 0;
}

void exprstack_pop_to_stop(exprstack_t *s)
{
    assert(s);

    struct exprstack_item *item = s->top;
    struct exprstack_item *tmp_item;
    enum exprstack_item_types type;

    // Empty stack
    if (!item)
        return;

    do {
        // Back up type of item to be deleted (we're looking for a STOP)
        type = item->type;
        tmp_item = item->prev;

        if (item->type == TERM && item->term_data.type == STRING)
            free(item->term_data.string);
        free(item);
        item = tmp_item;
    } while (item && type != STOP);

    s->top = item ? item : NULL;
}

token_t *exprstack_top_term(exprstack_t *s)
{
    assert(s);

    struct exprstack_item *item = s->top;

    // Empty stack
    if (!item)
        return NULL;

    while (item->type != TERM && item->prev)
        item = item->prev;

    if (item->type != TERM)
        return NULL;
    else
        return &item->term_data;
}

non_term_t *exprstack_top_non_term(exprstack_t *s)
{
    assert(s);

    struct exprstack_item *item = s->top;

    // Empty stack
    if (!item)
        return NULL;

    while (item->type != NON_TERM && item->prev)
        item = item->prev;

    if (item->type != NON_TERM)
        return NULL;
    else {
        // Set activity to the found item
        s->active = item;

        return &item->non_term_data;
    }
}

non_term_t *exprstack_next_non_term(exprstack_t *s)
{
    assert(s);
    assert(s->active);

    struct exprstack_item *item = s->active;

    // There is only one (currently active) non-term
    if (!item->prev)
        return NULL;
    else
        // Move to the next item (active one can't be used)
        item = item->prev;

    while (item->type != NON_TERM && item->prev)
        item = item->prev;

    if (item->type != NON_TERM)
        return NULL;
    else {
        // Update activity to the found item
        s->active = item;

        return &item->non_term_data;
    }
}

bool exprstack_check_top(exprstack_t *s, char *rule, ...)
{
    assert(s);
    assert(rule);

    int rule_length = (int) strlen(rule);
    struct exprstack_item *item = s->top;
    token_t stack_term;
    va_list terms;
    token_t terms_list[rule_length];

    // Function parameters needs to be reversed, because stack contains terminal reversed,
    // so we need to prepare them into array that could be iterated in the reverse order
    va_start(terms, rule);
    for (int i = 0; i < rule_length; i++) {
        // At the places where are terminals in the rule will be their values,
        // other places (where are non-terminals) are ignored
        if (rule[i] == 'T')
            terms_list[i] = va_arg(terms, token_t);
    }
    va_end(terms);

    // We need to go from the end of input rule, because stack contains it reversed
    for (int i = rule_length - 1; i >= 0; i--) {
        if (rule[i] == 'N') {
            if (item->type != NON_TERM)
                return false;
        }
        else if (rule[i] == 'T') {
            if (item->type != TERM)
                return false;

            stack_term = item->term_data;

            if (!token_equal(terms_list[i], stack_term))
                return false;
        } else {
            LOG_ERROR("Rule can contain only 'N' for non-terminal or 'T' for terminal. Found: %c", rule[i]);
            exit(EINTERNAL);
        }

        // Go to previous item of the stack
        if (!item->prev)
            return false;
        else
            item = item->prev;
    }

    // There must be STOP now, otherwise the rule on stack hasn't ended yet
    if (item->type != STOP)
        return false;

    return true;
}

bool exprstack_is_correctly_empty(exprstack_t *s)
{
    assert(s);

    struct exprstack_item *top = s->top;
    struct exprstack_item *bottom;

    // Completely empty stack (not valid for us, see details in doc comment)
    if (top)
        bottom = top->prev;
    else
        return false;

    // State: $N
    if (top->type == NON_TERM && bottom->type == TERM && bottom->term_data.type == END && bottom->prev == NULL)
        return true;

    return false;
}

void exprstack_destroy(exprstack_t *s)
{
    assert(s);

    struct exprstack_item *item = s->top;
    struct exprstack_item *tmp_item;

    while (item) {
        tmp_item = item->prev;

        free(item);
        item = tmp_item;
    }

    free(s);
}
