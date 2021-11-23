/**
 * @file kwtable.c
 * Table of keywords (language reserved symbols)
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */

#include "kwtable.h"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/**
 * Set of allowed keywords
 */
char *keywords[NUMBER_OF_KEYWORDS] = {
    "do", "else", "end", "function", "global", "if", "integer", "local", "nil",
    "number", "require", "return", "string", "then", "while"
};

kwtable_t *kwtable_create(void)
{
    kwtable_t *table = malloc(sizeof(kwtable_t));
    if (!table)
        return NULL;

    for (int i = 0; i < NUMBER_OF_KEYWORDS; i++) {
        (*table)[i] = keywords[i];
    }

    return table;
}

keyword_t *kwtable_find(kwtable_t *table, char *keyword_name)
{
    assert(table);

    int left = 0, right = NUMBER_OF_KEYWORDS - 1, middle;
    int compare_result;

    do {
        middle = (left + right) / 2;
        compare_result = strcmp(keyword_name, (*table)[middle]);

        if (compare_result < 0)
            right = middle - 1;
        else if(compare_result > 0)
            left = middle + 1;
        else
            return &((*table)[middle]);
    }
    while (right >= left);

    return NULL;
}

void kwtable_destroy(kwtable_t *table)
{
    assert(table);

    free(table);
}
