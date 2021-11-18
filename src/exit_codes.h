/**
 * @file exit_codes.h
 * IFJ21 compiler error numbers.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#ifndef _ERRNO_H_
#define _ERRNO_H_

/* Lexical analysis error. */
#define ELEX        1

/* Syntax analysis error. */
#define ESYNTAX     2

/* Not defined variable/function, redefinition etc. */
#define EDEF        3

/* Type incompatibility at assignment. */
#define EASSIGN     4

/* Incorrect arguments/return values when calling or returning from a function. */
#define EFUNCALL    5

/* Incompatible types in an expression. */
#define EEXPTYPE    6

/* Other semantic errors. */
#define ESEM_OTHER      7

/* Runtime error, unexpected nil where it shouldn't be. */
#define ENIL        8

/* Runtime error, division by zero. */
#define EZERODIV    9

/* Internal compiler error, failure to allocate memory etc. */
#define EINTERNAL   99

#endif

