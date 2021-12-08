/**
 * @file identifier.h
 * Header file of identifier
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 * @author Michal Šmahel (xsmahe01)
 */

#ifndef _IDENTIFIER_H_
#define _IDENTIFIER_H_

#include <stdbool.h>

enum variable_type {
    VAR_INTEGER='i', VAR_NUMBER='n', VAR_STRING='s', VAR_BOOL='b'
};
enum identifier_type {
    VARIABLE=1, FUNCTION
};

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
            bool called;
            char *param; // "" -> takes void
            char *retval; // "" -> returns void
        } fun;
    };
} identifier_t;

/**
 * Create a deep copy of identifier
 *
 * @details
 * Deep copy means all points contained in the identifier structure will be newly allocated
 * and original values copied to the new allocated space. You need to deallocate this copy
 * just like the original identifier.
 *
 * @param original_id Original identifier to create copy from
 * @return Pointer to the deep copy of original identifier
 */
identifier_t *identifier_clone(identifier_t *original_id);

#endif

