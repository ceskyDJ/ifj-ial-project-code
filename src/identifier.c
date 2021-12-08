/**
 * @file identifier.c
 * Identifier (variable or function) representation
 *
 * IFJ and IAL project (IFJ21 compiler)
 * Team: 128 (variant II)
 *
 * @author Martin Havlík (xhavli56)
 * @author Michal Šmahel (xsmahe01)
 */

#include "identifier.h"
#include "exit_codes.h"
#include <stdlib.h>
#include <string.h>

identifier_t *identifier_clone(identifier_t *original_id)
{
    identifier_t *new_id = malloc(sizeof(identifier_t));
    if (!new_id)
        exit(EINTERNAL);

    // Copy all contained information
    memcpy(new_id, original_id, sizeof(identifier_t));

    // Allocate new space for dynamic allocated data and copy values
    new_id->name = malloc(sizeof(original_id->name) + 1);
    strcpy(new_id->name, original_id->name);

    if (original_id->fun.param) {
        new_id->fun.param = malloc(sizeof(original_id->fun.param) + 1);
        strcpy(new_id->fun.param, original_id->fun.param);
    }

    if (original_id->fun.retval) {
        new_id->fun.retval = malloc(sizeof(original_id->fun.retval) + 1);
        strcpy(new_id->fun.retval, original_id->fun.retval);
    }

    return new_id;
}
