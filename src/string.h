/**
 * @file string_t.h
 * String_t abstract data type.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#ifndef _STRING_H_
#define _STRING_H_

#include <string.h>

#define INIT_MAX 16

/**
 * Abstract data type representing a string.
 */
typedef struct string string_t;

/**
 * Creates a new empty string.
 *
 * @param max Initial maximal length.
 *
 * @return  Pointer to newly created string ADT or
 *          NULL on creation failure.
 */
string_t *string_create(void);

/**
 * Destroys a string.
 *
 * @param str String to destroy.
 * @pre str != NULL
 */
void string_destroy(string_t *str);

/**
 * Appends a character to the string,
 *
 * @param str String to which to append to.
 * @param c Character to append.
 * @pre str != NULL
 *
 * @return 0 on successful append, negative value otherwise.
 */
int string_appendc(string_t *str, char c);

/**
 * Compares two strings.
 *
 * @param str1 First string to compare.
 * @param str2 Second string to compare.
 * @pre str1 != NULL
 * @pre str1->content != NULL
 * @pre str2 != NULL
 * @pre str2->content != NULL
 *
 * @return 0 if strings match, non-zero otherwise.
 */
int string_cmp(string_t *str1, string_t *str2);

/**
 * Creates `char *` without padding from the ADT string.
 *
 * @param str String from which to export.
 * @pre str != NULL
 *
 * @return `char *` containing the string without any padding.
 */
char *string_export(string_t *str);

struct string {
    size_t len;
    size_t max_len;
    char *content;
};

#endif

