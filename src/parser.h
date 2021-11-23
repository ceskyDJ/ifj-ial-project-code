/**
 * @file parser.h
 * Parser.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#ifndef _PARSER_H_
#define _PARSER_H_

#include "context.h"

/**
 * Creates all necessary structures and starts recursive descent
 * for top to bottom syntactical analysis.
 * Exits with code ESYNTAX on syntactical error.
 */
void parser_start(context_t *ctx);

#endif
