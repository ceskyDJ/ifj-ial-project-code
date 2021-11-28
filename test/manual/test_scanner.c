#include "../../src/context.h"
#include "../../src/token.h"
#include "../../src/scanner.h"
#include "../../src/symstack.h"
#include "../../src/string_factory.h"
#include "../../src/symtable.h"
#include <stdio.h>
#include <stdlib.h>

char *token_types[] = {
    "IDENTIFIER", "KEYWORD", "INTEGER", "NUMBER", "STRING", "ASSIGNMENT", "ADDITION", "SUBTRACTION",
    "MULTIPLICATION", "DIVISION", "INT_DIVISION", "CONCAT", "STRLEN", "LT", "GT", "LEQ", "GEQ", "EQ",
    "NEQ", "TYPE_SPEC", "COMMA", "LEFT_PAR", "RIGHT_PAR","END"
};

char *keywords[] = {
    "DO", "ELSE", "END", "FUNCTION", "GLOBAL", "IF", "INTEGER", "LOCAL", "NIL", "NUMBER", "REQUIRE",
    "RETURN", "STRING", "THEN", "WHILE"
};

int main() {
    context_t *context = malloc(sizeof(context_t));

    context->symstack = symstack_create();
    context->kwtable = kwtable_create();
    context->string = string_create();

    symtable_t *global_symtable = symtable_create();
    symstack_push(context->symstack, global_symtable);

    token_t token;
    identifier_t *identifier;
    do {
        token = get_next_token(context);

        printf("TOKEN = {\n  .type = %s", token_types[token.type]);

        switch (token.type) {
            case STRING:
                printf(",\n  .string = %s\n", token.string);
                break;
            case INTEGER:
                printf(",\n  .integer = %i\n", token.integer);
                break;
            case NUMBER:
                printf(",\n  .number = %f\n", token.number);
                break;
            case IDENTIFIER:
                identifier = token.identifier;
                printf(",\n  .identifier = {\n    .name = %s,\n    .character = %lu,\n    .line = %lu\n  }\n",
                        identifier->name, identifier->character, identifier->line);
                break;
            case KEYWORD:
                printf(",\n  .keyword = %s\n", keywords[*token.keyword]);
                break;
            default:
                puts("");
                break;
        }

        printf("}\n\n");
    } while (token.type != END);

    return 0;
}
