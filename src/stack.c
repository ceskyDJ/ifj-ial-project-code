/**
 * @file stack.c
 * Stack_t operations implementation.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Pavel Osinek (xosine00)
 */

#include "stack.h"

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>


void stack_init(stack_t *s)
{
    s->top = NULL;
    s->act = NULL;
    s->size = 0;
}

void stack_dispose(stack_t *s)
{
    assert(s);

    while(s->top != NULL) {
        stack_pop(s);
    }
}

int stack_push(stack_t *s, symtable_t *item)
{
    assert(s);
    assert(item);

    struct node_t *tmp = malloc(sizeof(struct node_t));
    if (tmp == NULL){
        return 1;
    }
    tmp->item = item;
    tmp->next = s->top;
    s->top = tmp;
    (s->size)++;

    return 0;
}

int stack_pop(stack_t *s)
{
    assert(s);

    struct node_t *tmp;
    if (s->top == NULL){
        return 1;
    }
    tmp = s->top;
    s->top = tmp->next;
    (s->size)--;

    free(tmp->item);
    free(tmp);

    return 0;
}

int first_active(stack_t *s)
{
    assert(s);

    if(s->top == NULL){
        return 1;
    }
    s->act = s->top;
    return 0;
}

int next_active(stack_t *s)
{
    assert(s);

    if(s->top == NULL){
        return 1;
    }
    if(s->act != NULL){
        if(s->act->next == NULL){
            s->act = NULL;
        }else if(s->act != NULL){
            s->act = s->act->next;
        }
    }
    return 0;
}

