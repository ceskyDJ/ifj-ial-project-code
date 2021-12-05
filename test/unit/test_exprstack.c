#include "../../unity/src/unity.h"
#include "../../src/exprstack.h"
#include "../../src/token.h"
#include <stdlib.h>

void test_exprstack_create(void)
{
    exprstack_t *s = exprstack_create();

    TEST_ASSERT_NOT_NULL(s);
}

void test_exprstack_top_term_empty(void)
{
    exprstack_t *s = exprstack_create();
    token_t *result;

    result = exprstack_top_term(s);

    TEST_ASSERT_NULL(result);
}

void test_exprstack_push_top_one_term(void)
{
    exprstack_t *s = exprstack_create();
    token_t term = {.type = MULTIPLICATION};
    token_t *result;

    exprstack_push_term(s, term);
    result = exprstack_top_term(s);

    TEST_ASSERT_EQUAL_INT(term.type, result->type);
}

void test_exprstack_push_top_more_terms(void)
{
    exprstack_t *s = exprstack_create();
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    token_t term3 = {.type = DIVISION};
    token_t *result;

    exprstack_push_term(s, term);
    exprstack_push_term(s, term2);
    exprstack_push_term(s, term3);
    result = exprstack_top_term(s);

    TEST_ASSERT_EQUAL_INT(term3.type, result->type);
}

void test_exprstack_add_stop_push_non_term_top_term(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    token_t *result;
    int return_code;

    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term);
    exprstack_push_non_term(s, non_term);
    return_code = exprstack_add_stop_after_top_term(s);
    result = exprstack_top_term(s);

    TEST_ASSERT_EQUAL_INT(return_code, 0);
    TEST_ASSERT_EQUAL_INT(term2.type, result->type);
}

void test_exprstack_add_stop_empty(void)
{
    exprstack_t *s = exprstack_create();
    int result;

    result = exprstack_add_stop_after_top_term(s);

    TEST_ASSERT_EQUAL_INT(-1, result);
}

void test_exprstack_pop_to_stop_empty(void)
{
    exprstack_t *s = exprstack_create();

    exprstack_pop_to_stop(s);

    TEST_PASS();
}

void test_exprstack_pop_to_stop_valid(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    token_t result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term);

    exprstack_pop_to_stop(s);

    // Unfortunately we must look in, because without it, it's hard
    // to check if the top of the stack has been updated successfully
    result = s->top->term_data;

    TEST_ASSERT_EQUAL_INT(term.type, result.type);
}

void test_exprstack_top_non_term_no_active(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    non_term_t *result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term);

    result = exprstack_top_non_term(s);

    TEST_ASSERT_EQUAL_INT(non_term.type, result->type);
}

void test_exprstack_top_non_term_missing(void)
{
    exprstack_t *s = exprstack_create();
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    non_term_t *result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_term(s, term2);

    result = exprstack_top_non_term(s);

    TEST_ASSERT_NULL(result);
}

void test_exprstack_top_non_term_active(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    non_term_t non_term2 = {.type = N_ERR};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    non_term_t *result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term2);
    exprstack_top_non_term(s);

    result = exprstack_top_non_term(s);

    TEST_ASSERT_EQUAL_INT(non_term2.type, result->type);
}

void test_exprstack_top_non_term_inside(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    non_term_t *result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term2);

    result = exprstack_top_non_term(s);

    TEST_ASSERT_EQUAL_INT(non_term.type, result->type);
}

void test_exprstack_next_non_term_missing(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    non_term_t *result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term2);
    exprstack_top_non_term(s);

    result = exprstack_next_non_term(s);

    TEST_ASSERT_NULL(result);
}

void test_exprstack_next_non_term_valid(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    non_term_t non_term2 = {.type = N_ERR};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    non_term_t *result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_non_term(s, non_term);
    exprstack_push_non_term(s, non_term2);
    exprstack_push_term(s, term2);
    exprstack_top_non_term(s);

    result = exprstack_next_non_term(s);

    TEST_ASSERT_EQUAL_INT(non_term.type, result->type);
}

void test_exprstack_next_non_term_more_calls(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    non_term_t non_term2 = {.type = N_ERR};
    non_term_t non_term3 = {.type = N_EXPR};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = INTEGER};
    non_term_t *result;
    non_term_t *result2;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_non_term(s, non_term);
    exprstack_push_non_term(s, non_term2);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term3);
    exprstack_top_non_term(s);

    result = exprstack_next_non_term(s);
    result2 = exprstack_next_non_term(s);

    TEST_ASSERT_EQUAL_INT(non_term2.type, result->type);
    TEST_ASSERT_EQUAL_INT(non_term.type, result2->type);
}

void test_exprstack_check_top_simple_valid(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = LEFT_PAR};
    token_t term3 = {.type = RIGHT_PAR};
    bool result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term3);

    result = exprstack_check_top(s, "TNT", term2, term3);

    TEST_ASSERT(result);
}

void test_exprstack_check_top_simple_invalid_token_type(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = LEFT_PAR};
    token_t term3 = {.type = RIGHT_PAR};
    bool result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term3);

    result = exprstack_check_top(s, "TNT", term3, term);

    TEST_ASSERT(!result);
}

void test_exprstack_check_top_simple_invalid_items_on_stack(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = LEFT_PAR};
    token_t term3 = {.type = RIGHT_PAR};
    bool result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term);
    exprstack_push_non_term(s, non_term);

    result = exprstack_check_top(s, "TNT", term3, term2);

    TEST_ASSERT(!result);
}

void test_exprstack_check_top_simple_more_items_on_stack(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};
    token_t term2 = {.type = LEFT_PAR};
    token_t term3 = {.type = RIGHT_PAR};
    bool result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term3);

    result = exprstack_check_top(s, "TNT", term3, term2);

    TEST_ASSERT(!result);
}

void test_exprstack_check_top_simple_invalid_keyword_val(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    keyword_t *kw1 = malloc(sizeof(keyword_t));
    *kw1 = KW_NIL;
    keyword_t *kw2 = malloc(sizeof(keyword_t));
    *kw2 = KW_DO;
    token_t term = {.type = KEYWORD, .keyword = kw1};
    token_t term2 = {.type = LEFT_PAR};
    token_t term3 = {.type = KEYWORD, .keyword = kw2};
    bool result;

    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);
    exprstack_push_term(s, term2);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term3);

    result = exprstack_check_top(s, "TNT", term, term2);

    TEST_ASSERT(!result);
}

void test_exprstack_is_correctly_empty_only_end_term(void)
{
    exprstack_t *s = exprstack_create();
    token_t term = {.type = END};
    bool result;

    exprstack_push_term(s, term);

    result = exprstack_is_correctly_empty(s);

    TEST_ASSERT(!result);
}

void test_exprstack_is_correctly_empty_end_term_non_term(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = END};
    bool result;

    exprstack_push_term(s, term);
    exprstack_push_non_term(s, non_term);

    result = exprstack_is_correctly_empty(s);

    TEST_ASSERT(result);
}

void test_exprstack_is_correctly_empty_completely_empty(void)
{
    exprstack_t *s = exprstack_create();
    bool result;

    result = exprstack_is_correctly_empty(s);

    TEST_ASSERT(!result);
}

void test_exprstack_is_correctly_empty_two_end_terms(void)
{
    exprstack_t *s = exprstack_create();
    token_t term = {.type = END};
    bool result;

    exprstack_push_term(s, term);
    exprstack_push_term(s, term);

    result = exprstack_is_correctly_empty(s);

    TEST_ASSERT(!result);
}

void test_exprstack_is_correctly_empty_more_non_terms(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = END};
    bool result;

    exprstack_push_term(s, term);
    exprstack_push_non_term(s, non_term);
    exprstack_push_non_term(s, non_term);
    exprstack_push_non_term(s, non_term);

    result = exprstack_is_correctly_empty(s);

    TEST_ASSERT(!result);
}

void test_exprstack_is_correctly_empty_many_items_in_there(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = END};
    token_t term2 = {.type = MULTIPLICATION};
    token_t term3 = {.type = LT};
    bool result;

    exprstack_push_term(s, term);
    exprstack_push_non_term(s, non_term);
    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term2);
    exprstack_push_term(s, term3);
    exprstack_push_non_term(s, non_term);

    result = exprstack_is_correctly_empty(s);

    TEST_ASSERT(!result);
}

void test_exprstack_destroy_empty(void)
{
    exprstack_t *s = exprstack_create();

    exprstack_destroy(s);

    TEST_PASS();
}

void test_exprstack_destroy_non_empty(void)
{
    exprstack_t *s = exprstack_create();
    non_term_t non_term = {.type = N_VAL};
    token_t term = {.type = MULTIPLICATION};

    exprstack_push_non_term(s, non_term);
    exprstack_push_term(s, term);
    exprstack_add_stop_after_top_term(s);

    exprstack_destroy(s);

    TEST_PASS();
}
