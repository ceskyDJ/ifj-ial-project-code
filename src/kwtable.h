/**
 * @file kwtable.h
 * Header file of table of keywords
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */

#ifndef _KWTABLE_H_
#define _KWTABLE_H_

/**
 * Keywords are predefined finite set, so we know its size
 * and can use it for defining static data structures, for example
 */
#define NUMBER_OF_KEYWORDS 15

/**
 * Keyword in integer-like form for simple comparing
 */
typedef enum keywords {
    KW_DO, KW_ELSE, KW_END, KW_FUNCTION, KW_GLOBAL, KW_IF, KW_INTEGER, KW_LOCAL,
    KW_NIL, KW_NUMBER, KW_REQUIRE, KW_RETURN, KW_STRING, KW_THEN, KW_WHILE
} keyword_t;

/**
 * Table of keywords
 */
typedef keyword_t kwtable_t[NUMBER_OF_KEYWORDS];

/**
 * Initializes a new keywords table (with filling)
 *
 * Initialized table is ready to use - it contains all available keywords.
 *
 * @return Pointer to new keywords table
 */
kwtable_t *kwtable_create(void);

/**
 * Searches for a keyword in the table
 *
 * @param table Keywords table to use
 * @param keyword_name Name of the keyword to search for
 * @return Pointer to keyword if found otherwise NULL
 *
 * @pre table != NULL
 */
keyword_t *kwtable_find(kwtable_t *table, char *keyword_name);

/**
 * Destroys keywords table
 *
 * @param table Table for destroying
 *
 * @pre table != NULL
 */
void kwtable_destroy(kwtable_t *table);

#endif //_KWTABLE_H_
