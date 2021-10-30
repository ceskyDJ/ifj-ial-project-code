/**
 * @file kwtable.h
 * Keyword table abstract data type.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#ifndef _KWTABLE_H_
#define _KWTABLE_H_

#include "symtable.h"

#define NO_KEYWORDS 15

/**
 * Keyword table abstract data type.
 */
typedef symtable_t kwtable_t;

/**
 * Type representing a keyword.
 */
typedef identifier_t keyword_t;

/**
 * Creates and fills table with keywords.
 *
 * @return  Pointer to new table or
 *          NULL on creation failure.
 */
kwtable_t *kwtable_create(void);

/**
 * Destroys an existing keyword table.
 *
 * @param table Pointer to table which to destroy.
 */
void kwtable_destroy(kwtable_t *table);

/**
 * Tries to find given identifier as a keyword in the table.
 *
 * @param table Keyword table in which to search.
 * @param id Identifier which we suspect of being a keyword.
 *
 * @return Pointer to the keyword that is the identifier or
 *          NULL if identifier is not a keyword.
 */
keyword_t *kwtable_find(kwtable_t *table, char *id);

#endif

