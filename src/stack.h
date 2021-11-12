/**
 * @file stack.c
 * Stack_t abstract data type implemented as singly linked list.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Pavel Osinek (xosine00)
 */

#ifndef _STACK_H
#define _STACK_H

#include "symtable.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * @struct Stack node
 */
typedef struct node_t {
        symtable_t *item;
            struct node_t *next;
} Node;

/**
 * @struct Stack
 *
 * ADT Stack implemented as a singly linked list.
 *
 */
typedef struct {
        Node * top;
            Node * act;
                unsigned size;
} stack_t;

/**
 * initializes stack to NULL
 *
 * @param s Stack pointer
 *
 */
void stack_init(stack_t *s);

/**
 * Disposes all nodes on the stack
 *
 * @param s Stack pointer
 *
 */
void stack_dispose(stack_t *s);

/**
 * Pushes data on top od the stack
 *
 * @param s Stack pointer
 * @param item Pointer to symtable_t
 * @return true if push completed successfully
 *
 */
int stack_push(stack_t *s, symtable_t *item);

/**
 * Removes data on top od the stack
 *
 * @param s Stack pointer
 * @return true if pop completed successfully
 *
 */
int stack_pop(stack_t *s);

/**
 * Function sets first stack node to active.
 *
 * @param s Stack pointer
 *
 */
int first_active(stack_t *s);

/**
 * Function sets next stack node to active.
 *
 * @param s Stack pointer
 *
 */
int next_active(stack_t *s);

#endif //_STACK_H

