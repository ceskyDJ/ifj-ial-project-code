/**
 * @file scanner.h
 * Header file of scanner (lexical analyzer)
 *
 * Scanner loads code from stdin and parses it into tokens for syntactic analyzer
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */

#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "token.h"
#include "symstack.h"
#include "context.h"

#include <stdbool.h>
#include <stdlib.h>

/**
 * Puts character back to standard input stream
 *
 * It doesn't put back whitespace characters except EOL
 */
#define UNGET_CHAR(c) ungetc(c, stdin)
/**
 * Calls string_append() function with error handling
 */
#define APPEND_C(string, c) do { if (string_appendc(string, c) < 0) exit(EINTERNAL); } while(0)
/**
 * Initializes token with type and "returns" it
 */
#define INIT_TOKEN(token, type_enum) token.type = type_enum, token
/**
 * Update coordinates for the new row (column is reset to the start of the row)
 */
#define NEW_ROW(row, col) do { (row)++; col = 0; } while(0)

/**
 * Obtains next token from standard input
 *
 * @param context Pointer to context with required dependencies
 * @return Parsed input encapsulated into token
 */
token_t get_next_token(context_t *context);

/**
 * Saves token to one-item cache
 *
 * This function provides functionality for "returning tokens to scanner".
 * Scanner saves the token into built-in cache and returns it at the
 * next calling of get_next_token() function. The cache is one-item only,
 * so you can put off just one token this way.
 *
 * @param token Token to be saved
 */
void unget_token(token_t token);

#endif //_SCANNER_H_
