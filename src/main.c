/**
 * @file main.c
 * Main control source file.
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Michal Šmahel (xsmahe01)
 */
#include <stdio.h>

#include "exit_codes.h"
#include "logger.h"
#include "parser.h"

int main() {

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

    // TODO fill global_symtable with builtin functions

    ret = symstack_push(symstack, global_symtable);
    if (ret < 0)
        exit(EINTERNAL);

    ctx.symstack = symstack;
    ctx.kwtable = kwtable;
    ctx.string = string;
    ctx.param = param;
    ctx.retval = retval;

    parser_start(&ctx);

    // check global symtable is the last remaining
    if (symstack_pop(symstack) != global_symtable) {
        LOG_ERROR_M("Global symtable is NOT the only remaining one!");
        while (symstack_pop(symstack)) {
            LOG_ERROR_M("Popped remaining local symtable.");
        }
    }

    // TODO free param and retval for id's in global symtable

    string_destroy(retval);
    string_destroy(param);
    string_destroy(string);
    kwtable_destroy(kwtable);
    symtable_destroy(global_symtable);
    symstack_destroy(symstack);

    return 0;
}
