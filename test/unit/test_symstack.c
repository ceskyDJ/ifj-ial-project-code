#include "../../unity/src/unity.h"
#include "../../src/symstack.h"
#include "../../src/symtable.h"

void test_symstack_init(void)
{
    symstack_t *symstack = symstack_create();

    TEST_ASSERT_NOT_NULL(symstack);
}

void test_symstack_push_one(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t = symtable_create();

    symstack_push(symstack, t);

    symstack_most_local(symstack);
    result = symstack_get(symstack);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(t, result);
}

void test_symstack_push_more(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();
    symtable_t *t3 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);
    symstack_push(symstack, t3);

    symstack_most_local(symstack);
    result = symstack_get(symstack);
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(t3, result);
}

void test_symstack_pop_empty(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;

    result = symstack_pop(symstack);

    TEST_ASSERT_NULL(result);
}

void test_symstack_pop_one_item_symstack(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t = symtable_create();

    symstack_push(symstack, t);
    result = symstack_pop(symstack);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(t, result);
}

void test_symstack_pop_more_items_symstack(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();
    symtable_t *t3 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);
    symstack_push(symstack, t3);

    result = symstack_pop(symstack);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(t3, result);
}

void test_symstack_pop_more_items_from_more_items_symstack(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();
    symtable_t *t3 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);
    symstack_push(symstack, t3);

    symstack_pop(symstack);
    result = symstack_pop(symstack);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(t2, result);
}

void test_symstack_top_empty(void)
{
    symstack_t *symstack = symstack_create();

    symstack_most_local(symstack);

    TEST_PASS();
}

void test_symstack_get_empty(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;

    result = symstack_get(symstack);

    TEST_ASSERT_NULL(result);
}

void test_symstack_next_empty(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;

    symstack_most_local(symstack);

    symstack_next(symstack);
    result = symstack_get(symstack);

    TEST_ASSERT_NULL(result);
}

void test_symstack_next_classic(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);
    symstack_most_local(symstack);

    symstack_next(symstack);
    result = symstack_get(symstack);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(t1, result);
}

void test_symstack_next_overflow(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t1 = symtable_create();

    symstack_push(symstack, t1);
    symstack_most_local(symstack);

    symstack_next(symstack);
    result = symstack_get(symstack);

    TEST_ASSERT_NULL(result);
}

void test_symstack_global_symtable_empty(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;

    result = symstack_global_symtable(symstack);

    TEST_ASSERT_NULL(result);
}

void test_symstack_global_symtable_one_table(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t1 = symtable_create();

    symstack_push(symstack, t1);

    result = symstack_global_symtable(symstack);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(t1, result);
}

void test_symstack_global_symtable_more_tables(void)
{
    symstack_t *symstack = symstack_create();
    symtable_t *result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();
    symtable_t *t3 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);
    symstack_push(symstack, t3);

    result = symstack_global_symtable(symstack);

    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(t1, result);
}

void test_symstack_is_active_empty(void)
{
    symstack_t *symstack = symstack_create();
    bool result;

    result = symstack_is_active(symstack);

    TEST_ASSERT(!result);
}

void test_symstack_is_active_non_empty_not_active(void)
{
    symstack_t *symstack = symstack_create();
    bool result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);

    result = symstack_is_active(symstack);

    TEST_ASSERT(!result);
}

void test_symstack_is_active_non_empty_active(void)
{
    symstack_t *symstack = symstack_create();
    bool result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);
    symstack_most_local(symstack);

    result = symstack_is_active(symstack);

    TEST_ASSERT(result);
}

void test_symstack_is_active_pop_active_table(void)
{
    symstack_t *symstack = symstack_create();
    bool result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);
    symstack_most_local(symstack);
    symstack_pop(symstack);

    result = symstack_is_active(symstack);

    TEST_ASSERT(!result);
}

void test_symstack_is_active_next_overflow(void)
{
    symstack_t *symstack = symstack_create();
    bool result;
    symtable_t *t1 = symtable_create();
    symtable_t *t2 = symtable_create();

    symstack_push(symstack, t1);
    symstack_push(symstack, t2);
    symstack_most_local(symstack);
    symstack_next(symstack);
    symstack_next(symstack);

    result = symstack_is_active(symstack);

    TEST_ASSERT(!result);
}

void test_symstack_destroy(void)
{
    symstack_t *symstack = symstack_create();

    symstack_destroy(symstack);

    TEST_PASS();
}
