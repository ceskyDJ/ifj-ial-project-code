/**
 * @file symqueue.c
 * ADT Queue for code generator implementation.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Pavel Osinek (xosine00)
 */

#include "symqueue.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

symqueue_t *symqueue_create(void)
{
    symqueue_t *q = malloc(sizeof(symqueue_t));
    if (!q) {
        return NULL;
    }

    q->front = NULL;
    q->rear = NULL;

    return q;
}

bool symqueue_add(symqueue_t *q, identifier_t *item)
{
    assert(q);
    assert(item);

    struct symqueue_node *tmp = malloc(sizeof(struct symqueue_node));
    if (!tmp) {
        return -ENOMEM;
    }
    tmp->item = item;
    if (q->front == NULL) {
        q->front = tmp;
        q->rear = tmp;
        q->front->next = NULL;
    } else {
        q->rear->next = tmp;
        q->rear = tmp;
        q->rear->next = NULL;
    }
    return true;
}

identifier_t *symqueue_peek(symqueue_t *q)
{
    assert(q);

    if (q->front == NULL) {
        return NULL;
    }
    
    return q->front->item;
}

identifier_t *symqueue_rear(symqueue_t *q)
{
    assert(q);

    if (q->rear == NULL) {
        return NULL;
    }
    return q->rear->item;
}

identifier_t *symqueue_pop(symqueue_t *q)
{
    assert(q);

    struct symqueue_node *tmp;
    identifier_t *item;

    if (q->front == NULL) {
        return NULL; //empty queue
    } else {
        tmp = q->front;
        item = tmp->item;
        q->front = q->front->next;
        
        if (q->front == NULL) {
            q->rear = NULL;
        }
        
        free(tmp);
        return item;
    }
}

bool symqueue_is_empty(symqueue_t *q)
{
    assert(q);

    return (q->front == NULL);
}

void symqueue_destroy(symqueue_t *q)
{
    assert(q);

    free(q);
}
