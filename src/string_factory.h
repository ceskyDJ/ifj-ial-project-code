/**
 * @file string_factory.h
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
 * @return  Pointer to newly created string ADT or
 *          NULL on creation failure.
 */
string_t *string_create(void);

/**
 * Destroys a string.
 *
 * @param str String to destroy.
 *
 * @pre str != NULL
 */
void string_destroy(string_t *str);

/**
 * Appends a character to the string,
 *
 * @param str String to which to append to.
 * @param c Character to append.
 *
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
 *
 * @pre str1 != NULL
 * @pre str1->content != NULL
 * @pre str2 != NULL
 * @pre str2->content != NULL
 *
 * @return 0 if strings match, non-zero otherwise.
 */
int string_cmp(string_t *str1, string_t *str2);

/**
 * Checks if first string is the prefix of the second string.
 * Any string contains an empty prefix.
 *
 * @param str1 String that should be the prefix.
 * @param str2 String that should containt str1 as prefix.
 *
 * @pre str1 != NULL
 * @pre str2 != NULL
 *
 * @return 0 if str1 is indeed a prefix of str2, non-zero otherwise
 */
int string_check_prefix(string_t *str1, string_t *str2);

/**
 * Allocates and fills memory with the content of ADT string
 * without any padding. Returned `char *` needs to be freed!
 *
 * @param str String from which to export.
 *
 * @pre str != NULL
 *
 * @return `char *` containing the string without any padding.
 */
char *string_export(string_t *str);

/**
 * Exposes the content of ADT string for uses that do not need
 * allocated memory to work with.
 *
 * @param str String which to expose the content of.
 *
 * @pre str != NULL
 *
 * @return `char *` of the string content
 */
char *string_expose(string_t *str);

/**
 * Clears (zeroes out) ADT string for safe repeated use.
 *
 * @param str String which to clear.
 *
 * @pre str != NULL
 */
void string_clear(string_t *str);

struct string {
    size_t len;
    size_t max_len;
    char *content;
};

#endif

