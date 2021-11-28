#include "../../unity/src/unity.h"
#include "../../src/scanner.h"

void test_unget_token_true_usage(void)
{
    context_t *context = malloc(sizeof(context_t));

    context->symstack = symstack_create();
    context->kwtable = kwtable_create();
    context->string = string_create();

    token_t token = {.type = END};
    token_t result;

    unget_token(token);

    result = get_next_token(context);

    TEST_ASSERT_EQUAL_INT(token.type, result.type);
}

void test_unget_token_bad_usage(void)
{
    token_t token1 = {.type = END};
    token_t token2 = {.type = GT};

    unget_token(token1);
    unget_token(token2);

    // It should not fall on error
    TEST_PASS();
}
