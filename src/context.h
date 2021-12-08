/**
 * @file context.h
 * Context for lexical and syntactical analysis.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 */

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "symstack.h"
#include "kwtable.h"
#include "string_factory.h"
#include "symqueue.h"

/**
 * Context for lexical and syntactical analysis
 *
 * Content:
 * <ul>
 *      <li><code>symstack</code> - Pointer to the stack-like storage of tables of symbols</li>
 *      <li><code>kwtable</code> - Pointer to keywords table</li>
 *      <li><code>string</code> - Pointer to temporary string of string factory</li>
 * </ul>
 */
typedef struct context {
    symstack_t *symstack;
    kwtable_t *kwtable;
    string_t *string;
    identifier_t *saved_id;
    string_t *param;
    string_t *retval;
    symqueue_t *main_symqueue;
    symqueue_t *cycle_symqueue;
} context_t;

#endif // _CONTEXT_H_
