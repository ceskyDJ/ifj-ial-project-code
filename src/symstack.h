/**
 * @file symstack.c
 * Header of stack-like storage for tables of symbols
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Pavel Osinek (xosine00)
 * @author Michal Šmahel (xsmahe01)
 */

#ifndef _SYMSTACK_H_
#define _SYMSTACK_H_

#include "symtable.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/**
 * Stack-like structure is internally implemented as linked-list,
 * so we need to store pointers for next items somewhere.
 *
 * Pointer to next item points to the more-global table of symbols.
 * The first node is activated by symstack_most_local() function and points
 * to the most-local (currently processed) scope, its `next` pointer
 * points to the previous scope (higher) etc.
 */
struct symstack_node {
    symtable_t *item;
    struct symstack_node *next;
};

/**
 * Stack-like structure for storing tables of symbols implemented as linked list
 * It's realized throw top and active pointers. Active pointer (structure's "list
 * part") is set by symstack_most_local() and symstack_next() functions. Top is the top
 * of the "stack part" of the structure.
 */
struct symstack {
    struct symstack_node *top;
    struct symstack_node *act;
};

/**
 * Stack-like ADT for storing pointers to tables of symbols
 */
typedef struct symstack symstack_t;

/**
 * Creates a new stack-like structure for storing symtable records (symstack)
 *
 * @return Pointer to created symstack
 */
 symstack_t *symstack_create(void);

/**
 * Pushes new table of symbols to the top of the "stack"
 *
 * @param s Pointer to symstack
 * @param symtable Pointer to symtable_t to be pushed
 * @return 0 on successful push, negative value otherwise
 *
 * @pre s != NULL
 * @pre symtable != NULL
 */
int symstack_push(symstack_t *s, symtable_t *symtable);

/**
 * Takes out the table of symbols on the top of the "stack"
 *
 * This function deletes the table pointer from the symstack!
 *
 * @param s Pointer to symstack
 * @return Symbols table on the top of the "stack" or NULL if it's empty
 *
 * @pre s != NULL
 */
symtable_t *symstack_pop(symstack_t *s);

/**
 * Sets the most-local symbols table on the top of "stack" as active
 *
 * @param s Pointer to symstack
 *
 * @pre s != NULL
 */
void symstack_most_local(symstack_t *s);

/**
 * Sets the more-global symbols table as active
 *
 * @param s Pointer to symstack
 *
 * @pre s != NULL
 */
void symstack_next(symstack_t *s);

/**
 * Returns active symtable
 *
 * @param s Pointer to symstack
 * @return Pointer to the active symtable or NULL if no symtable is active
 *
 * @pre s != NULL
 */
symtable_t *symstack_get(symstack_t *s);

/**
 * Returns global symtable stored in the symstack
 *
 * @param s Pointer to symstack
 * @return Global symtable or NULL if symstack is empty
 *
 * @pre s != NULL
 */
symtable_t *symstack_global_symtable(symstack_t *s);

/**
 * Checks if the activity flag is set to one of stored tables of symbols
 *
 * @param s Pointer to symstack
 * @return Is there any active node?
 *
 * @pre s != NULL
 */
bool symstack_is_active(symstack_t *s);

/**
 * Destroys empty symstack
 *
 * @param s Symstack to be destroyed
 *
 * @pre s != NULL
 * @pre symstack_most_local(s); symstack_is_active(s) == false
 */
void symstack_destroy(symstack_t *s);

#endif //_SYMSTACK_H_
