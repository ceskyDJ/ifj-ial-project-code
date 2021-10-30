/**
 * @file string_t.c
 * String_t operations implementation.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#include "string.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

string_t *string_create(void)
{
    string_t *string = malloc(sizeof(string_t));
    if (!string)
        return NULL;

    string->content = malloc(INIT_MAX);
    if (!string->content) {
        free(string);
        return NULL;
    }

    string->len = 0;
    string->max_len = INIT_MAX;

    return string;
}

void string_destroy(string_t *str)
{
    assert(str);

    free(str->content);
    free(str);
}

int string_appendc(string_t *str, char c)
{
    assert(str);
    char *grown;

    if (str->len == str->max_len - 1) {
        grown = calloc(2*str->max_len, 1);
        if (!grown) {
            return -errno;
        }
        str->max_len = 2*str->max_len;

        strncpy(grown, str->content, str->len);

        free(str->content);
        str->content = grown;
    }

    str->content[str->len++] = c;

    return 0;
}

int string_cmp(string_t *str1, string_t *str2)
{
    assert(str1);
    assert(str2);

    return strcmp(str1->content, str2->content);
}

char *string_export(string_t *str)
{
    assert(str);

    char *export = calloc(str->len + 1, sizeof(char));

    if (!export)
        return NULL;

    strncpy(export, str->content, str->len);

    return export;
}

