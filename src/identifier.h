/**
 * @file identifier.h
 * Identifier structure.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#ifndef _IDENTIFIER_H_
#define _IDENTIFIER_H_

enum variable_type {
    VAR_INTEGER='i', VAR_NUMBER='n', VAR_STRING='s'
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
            char *param; // "" -> takes void
            char *retval; // "" -> returns void
        } fun;
    };
} identifier_t;

#endif

