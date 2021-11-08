/**
 * @file symtable.h
 * Symbol table abstract data type.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */
#ifndef _SYMTABLE_H_
#define _SYMTABLE_H_

#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#define SYMTABLE_BUCKETS 23

enum variable_type {
    INTEGER='i', NUMBER='n', STRING='s'
};
enum identifier_type {
    VARIABLE=1, FUNCTION
};

/**
 * Symbol table abstract data type.
 */
typedef struct symtable symtable_t;


/**
 * Structure representing an identifier,
 * an element of symtable.
 */
typedef struct identifier {
    char *name;
    unsigned long line;
    unsigned long character;
    enum identifier_type type;
    union {
        struct variable {
            char type;
            int init;
            int used;
        } var;
        struct function {
            int defined;
            char *param; // "" -> takes void
            char *retval; // "" -> returns void
        } fun;
    };
} identifier_t;

typedef struct symtable_item {
    identifier_t identifier;
    struct symtable_item *next;
} symtable_item_t;

/**
 * Structure representing symtable.
 *
 * size     the current number of identifiers in the symtable
 * buckets  the on-creation determined number of buckets
 * items    array of pointers to items, each being the start of corresponding bucket
 */
struct symtable {
    size_t size;
    size_t buckets;
    struct symtable_item *items[];
};

/**
 * Creates an empty new table.
 *
 * @return  Pointer to newly created table.
 *          Null on creation failure.
 */
symtable_t *symtable_create(void);

/**
 * Destroys a table, desposing of all elements in it.
 *
 * @param t Table to destroy.
 */
void symtable_destroy(symtable_t *t);

/**
 * Tries to find an identifier with matching name.
 *
 * @param t Table to find the identifier in.
 * @param name Name of the identifier to find.
 *
 * @return  Pointer to identifier with matching name or
 *          Null if matching name is not found.
 */
identifier_t *symtable_find(symtable_t *t, char *name);

/**
 * Adds an identifier to table if it is not already present.
 *
 * @param t Table to add the identifier to.
 * @param name Name of the identifier to add.
 *
 * @return  Pointer to the already presest identifier or
 *          pointer to the newly added identifier.
 */
identifier_t *symtable_add(symtable_t *t, char *name);

#endif

