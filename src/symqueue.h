/**
 * @file symqueue.h
 * ADT Queue for code generator.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Pavel Osinek (xosine00) 
 */

#ifndef _SYMQUEUE_H
#define _SYMQUEUE_H

#include "symtable.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @struct Symqueue_node
 */
struct symqueue_node {
    identifier_t *item;
    struct symqueue_node *next;
};

/**
 * @struct Symqueue
 *
 * ADT Queue for code generator implemented as singly linked list
 *
 */
struct symqueue {
    struct symqueue_node *front;    //first in item
    struct symqueue_node *rear;     //last in item
};

typedef struct symqueue symqueue_t;

/**
 * Creates a new symqueue
 *
 * @return Pointer to new symqueue
 */
symqueue_t *symqueue_create(void);

/**
 * Adds new symtable item to symqueue as last in item
 *
 * @param q symqueue
 * @return If add was successful
 *
 * @pre q != NULL
 * @pre item != NULL
 */
bool symqueue_add(symqueue_t *q, identifier_t *item);

/**
 * Returns first in symtable item without removing it
 *
 * @param q symqueue
 * @return pointer to first in symtable item
 *
 * @pre q != NULL
 */
identifier_t *symqueue_peek(symqueue_t *q);

/**
 * Returns last in symtable item without removing it
 *
 * @param q symqueue
 * @return pointer to last in symtable item
 *
 * @pre q != NULL
 */
identifier_t *symqueue_rear(symqueue_t *q);

/**
 * Returns and pops first in symtable item
 *
 * @param q symqueue
 * @return pointer to poped first in symtable item
 *
 * @pre q != NULL
 */
identifier_t *symqueue_pop(symqueue_t *q);

/**
 * Checks if symqueue is empty
 *
 * @param q symqueue
 * @return Is there any node?
 *
 * @pre q != NULL
 */
bool symqueue_is_empty(symqueue_t *q);

/**
 * Destroys empty symqueue
 *
 * @param q symqueue
 *
 * @pre q != NULL; symqueue_is_empty(q) == true
 */
void symqueue_destroy(symqueue_t *q);

#endif
