#include "../../src/expr_parser.h"
#include "../../src/logger.h"

int main(void)
{
    context_t *context = malloc(sizeof(context_t));
    enum variable_type result;

    context->symstack = symstack_create();
    context->kwtable = kwtable_create();
    context->string = string_create();

    symtable_t *global_symtable = symtable_create();
    symstack_push(context->symstack, global_symtable);

    result = expr_parser_start(context);

    switch (result) {
        case VAR_INTEGER:
            LOG_DEBUG_M("Final expression type: integer");
            break;
        case VAR_NUMBER:
            LOG_DEBUG_M("Final expression type: number");
            break;
        case VAR_STRING:
            LOG_DEBUG_M("Final expression type: string");
            break;
        case VAR_BOOL:
            LOG_DEBUG_M("Final expression type: bool");
            break;
        default:
            LOG_ERROR_M("Expressions parser returned something unknown");
            break;
    }

    return 0;
}
