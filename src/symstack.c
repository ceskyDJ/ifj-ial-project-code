/**
 * @file stack.c
 * Stack-like storage for tables of symbols
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Pavel Osinek (xosine00)
 * @author Michal Šmahel (xsmahe01)
 */

#include "symstack.h"
#include "symtable.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>

symstack_t *symstack_create(void)
{
    symstack_t *s = malloc(sizeof(symstack_t));
    if (!s)
        return NULL;

    s->top = NULL;
    s->act = NULL;

    return s;
}

int symstack_push(symstack_t *s, symtable_t *symtable)
{
    assert(s);
    assert(symtable);

    // Allocate new node for storing symtable
    struct symstack_node *tmp = malloc(sizeof(struct symstack_node));
    if (!tmp)
        return -ENOMEM;

    // Initialize new node
    tmp->item = symtable;
    tmp->next = s->top;
    s->top = tmp;

    return 0;
}

symtable_t *symstack_pop(symstack_t *s)
{
    assert(s);

    struct symstack_node *top_node;
    symtable_t *symtable;

    if (!s->top)
        // Symstack is empty
        return NULL;

    if (s->top == s->act)
        // Active symtable will be removed --> it can't be active anymore
        s->act = NULL;

    // Extract symtable from the top node
    top_node = s->top;
    symtable = top_node->item;

    // Delete top node from the symstack
    s->top = top_node->next;
    free(top_node);

    return symtable;
}

void symstack_most_local(symstack_t *s)
{
    assert(s);

    s->act = s->top;
}

void symstack_next(symstack_t *s)
{
    assert(s);

    if (s->act)
        s->act = s->act->next;
}

symtable_t *symstack_get(symstack_t *s)
{
    assert(s);

    if (s->act)
        return s->act->item;
    else
        return NULL;
}

symtable_t *symstack_global_symtable(symstack_t *s)
{
    assert(s);

    struct symstack_node *current_node = s->top;

    if (!current_node)
        // Symstack is empty
        return NULL;

    // Find oldest symtable (it's the global one)
    while (current_node->next != NULL)
        current_node = current_node->next;

    return current_node->item;
}

bool symstack_is_active(symstack_t *s)
{
    assert(s);

    return (s->act);
}

void symstack_destroy(symstack_t *s)
{
    assert(s);

    free(s);
}
