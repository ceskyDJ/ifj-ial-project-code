/**
 * @file main.c
 * Main control source file.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 * @author Martin Havlík (xhavli56)
 */
#include <stdio.h>

#include "exit_codes.h"
#include "logger.h"
#include "parser.h"
#include "symtable.h"
#include "identifier.h"
#include "string.h"

static void add_builtin_function(symtable_t *symtable, char *name, char *param, char *retval)
{
    identifier_t *fun;
    char *param_alloc;
    char *retval_alloc;

    param_alloc = malloc(strlen(param) + 1);
    retval_alloc = malloc(strlen(retval) + 1);
    if (!param_alloc || !retval_alloc)
        exit(EINTERNAL);

    strcpy(param_alloc, param);
    strcpy(retval_alloc, retval);

    fun = symtable_add(symtable, name);
    fun->type = FUNCTION;
    fun->line = 0;
    fun->character = 0;
    fun->fun.defined = true;
    fun->fun.param = param_alloc;
    fun->fun.retval = retval_alloc;
}

static void init_builtin_functions(symtable_t *symtable)
{
    add_builtin_function(symtable, "reads", "", "s");
    add_builtin_function(symtable, "readi", "", "i");
    add_builtin_function(symtable, "readn", "", "n");
    add_builtin_function(symtable, "write", "write", "");
    add_builtin_function(symtable, "tointeger", "n", "i");
    add_builtin_function(symtable, "substr", "snn", "s");
    add_builtin_function(symtable, "ord", "si", "i");
    add_builtin_function(symtable, "chr", "i", "s");
}

int main()
{
    context_t ctx;
    int ret;

    symstack_t *symstack = symstack_create();
    if (!symstack)
        exit(EINTERNAL);

    symtable_t *global_symtable = symtable_create();
    if (!global_symtable)
        exit(EINTERNAL);

    kwtable_t *kwtable = kwtable_create();
    if (!kwtable)
        exit(EINTERNAL);

    string_t *string = string_create();
    if (!string)
        exit(EINTERNAL);

    string_t *param = string_create();
    if (!param)
        exit(EINTERNAL);

    string_t *retval = string_create();
    if (!retval)
        exit(EINTERNAL);

    symqueue_t *main_symqueue = symqueue_create();
    if (!main_symqueue)
        exit(EINTERNAL);

    symqueue_t *cycle_symqueue = symqueue_create();
    if (!cycle_symqueue)
        exit(EINTERNAL);

    init_builtin_functions(global_symtable);

    ret = symstack_push(symstack, global_symtable);
    if (ret < 0)
        exit(EINTERNAL);

    ctx.symstack = symstack;
    ctx.kwtable = kwtable;
    ctx.string = string;
    ctx.param = param;
    ctx.retval = retval;
    ctx.main_symqueue = main_symqueue;
    ctx.cycle_symqueue = cycle_symqueue;

    parser_start(&ctx);

    // check global symtable is the last remaining
    if (symstack_pop(symstack) != global_symtable) {
        LOG_ERROR_M("Global symtable is NOT the only remaining one!");
        while (symstack_pop(symstack)) {
            LOG_ERROR_M("Popped remaining local symtable.");
        }
    }

    string_destroy(retval);
    string_destroy(param);
    string_destroy(string);
    kwtable_destroy(kwtable);
    symtable_destroy(global_symtable);
    symstack_destroy(symstack);
    while (!symqueue_is_empty(main_symqueue))
        symqueue_pop(main_symqueue);
    symqueue_destroy(main_symqueue);
    while (!symqueue_is_empty(cycle_symqueue))
        symqueue_pop(cycle_symqueue);
    symqueue_destroy(cycle_symqueue);
    return 0;
}
