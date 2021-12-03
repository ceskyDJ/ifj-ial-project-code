/**
 * @file token.h
 * Token structure.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 * @author Michal Šmahel (xsmahe01)
 */

#ifndef _TOKEN_H_
#define _TOKEN_H_

#include "symtable.h"
#include "kwtable.h"

enum token_type {
    // Symbols
    IDENTIFIER, KEYWORD,
    // Literals
    INTEGER, NUMBER, STRING, BOOL,
    // Assignment operators
    ASSIGNMENT,
    // Math operators
    ADDITION, SUBTRACTION, MULTIPLICATION, DIVISION, INT_DIVISION,
    // String operators
    CONCAT, STRLEN,
    // Conditional operators
    LT, GT, LEQ, GEQ, EQ, NEQ,
    // Special operators
    TYPE_SPEC, COMMA,
    // Other
    LEFT_PAR, RIGHT_PAR,
    // End of input, last token to be sent
    END
};

typedef struct token {
    enum token_type type;
    union {
        int integer;
        double number;
        char *string;
        identifier_t *identifier;
        keyword_t *keyword;
    };
} token_t;

#endif

