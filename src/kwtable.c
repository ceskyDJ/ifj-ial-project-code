/**
 * @file kwtable.c
 * Keyword table operations implementation.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#include "kwtable.h"
#include "symtable.h"

#include <stdlib.h>
#include <string.h>

char *keywords[NO_KEYWORDS] = { "do", "else", "end", "function", "global", "if",
                            "local", "nil", "require", "return", "then", "while",
                            "integer", "string", "number"};

kwtable_t *kwtable_create(void)
{
    symtable_t *t = symtable_create();
    keyword_t *kw;

    if (!t)
        return NULL;

    for (int i = 0; i < NO_KEYWORDS; i++) {
        kw = (keyword_t *) symtable_add(t, keywords[i]);
        if (!kw) {
            symtable_destroy(t);
            return NULL;
        }
    }

    return (kwtable_t *) t;
}

void kwtable_destroy(kwtable_t *table)
{
    symtable_destroy((symtable_t *)table);
}

char *kwtable_find(kwtable_t *table, char *id)
{
    keyword_t *kw;

    kw = (keyword_t*) symtable_find(table, id);

    return kw ? kw->name : NULL;
}
