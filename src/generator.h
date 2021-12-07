/*
 * @file generator.h
 * IFJcode21 generator.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#ifndef _GENERATOR_H_
#define _GENERATOR_H_

#include <stdbool.h>

#include "token.h"
#include "symtable.h"
#include "symqueue.h"

/*
 * IFJcode21 symbol name prefixes:
 * <no prefix>  ... normal local variable
 *      $       ... compiler variable
 *      %       ... variable for moving value between caller and callee
 *      &       ... label
 */
/* I/O data convention:
 * %n - n-th input parameter
 * %retval_n - n-th output parameter
 * %params - number of parameters (for functions with variable number of parameters)
 *
 * For functions with variable number of parameters input parameters are passed on the stack,
 * the first parameter is on the top (they are pushed in the reversed order)
 */

/**
 */
void gen_builtins(void);

/**
 * Generates required `.IFJcode21` header.
 */
void gen_ifjcode21(void);

void gen_fun_start(identifier_t *id);

void gen_fun_param(identifier_t *id);

void gen_create_frame();

/**
 * @param conv_to_number If implicit conv should be generated for the param.
 */
void gen_call_param(token_t *token, bool conv_to_number);

void gen_call(identifier_t *id);

void gen_returned_assign(symqueue_t *queue);

void gen_var_dec(identifier_t *id, symqueue_t *queue);

void gen_var_dec_assign(symqueue_t *queue, bool value_on_stack);

void gen_var_set_active(identifier_t *id, symqueue_t *queue);

void gen_var_active_assign(symqueue_t *queue, bool value_on_stack);

void gen_var_retval(void);

void gen_fun_end(identifier_t *id);

/**
 * Pushes term (id/integer/number/string/nil) to stack.
 */
void gen_push_term(token_t *token);

/**
 * Makes said operation, expects operand(s) on stack.
 */
void gen_operation(enum token_type type);

/**
 * Expects expression result (even non bool) at the top of the stack
 */
unsigned int gen_if_start(void);

void gen_if_else(unsigned int if_cnt);

void gen_if_end(unsigned int if_cnt);

// TODO in while we want skipping DEFVARs, implement it
void gen_while_start_before_expr(void);

void gen_while_start_after_expr(void);

void gen_while_end(void);

/**
 * Runtime nil check for term TOP on stack.
 */
void gen_nil_check_top(void);

/**
 * Runtime nil check for ("previous") term JUST UNDER TOP of stack.
 */
void gen_nil_check_subtop(void);

/**
 * Runtime zero check for top operand (the second one in operations).
 */
void gen_zero_div_check(void);

/**
 * Type conversion of the term TOP on stack.
 */
void gen_conv_to_number_top(void);

/**
 * Type conversion of the term TOP on stack.
 */
void gen_conv_to_number_subtop(void);

void gen_write_identifier(identifier_t *id);

void gen_write_integer(int i);

void gen_write_number(double n);

void gen_write_string(char *s);

void gen_write_nil(void);

#endif

