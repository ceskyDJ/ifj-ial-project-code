#include "../../unity/src/unity.h"
#include "../../src/symqueue.h"
#include "../../src/symtable.h"

void test_symqueue_create(void)
{
    symqueue_t *symqueue = symqueue_create();
    
    TEST_ASSERT_NOT_NULL(symqueue);
}

void test_symqueue_add_one(void)
{
    symqueue_t *symqueue = symqueue_create();
    symtable_t *st = symtable_create();
    identifier_t *item = symtable_add(st, "aaa");
    identifier_t *res1;
    identifier_t *res2;
    
    symqueue_add(symqueue, item);
    res1 = symqueue_peek(symqueue);
    res2 = symqueue_rear(symqueue);
    
    TEST_ASSERT_NOT_NULL(res1);
    TEST_ASSERT_NOT_NULL(res2);
    TEST_ASSERT_EQUAL_PTR(item, res1);
    TEST_ASSERT_EQUAL_PTR(item, res2);
}

void test_symqueue_add_more(void)
{
    symqueue_t *symqueue = symqueue_create();
    symtable_t *st = symtable_create();
    
    identifier_t *item1 = symtable_add(st, "aaa");
    identifier_t *item2 = symtable_add(st, "bbb");
    identifier_t *item3 = symtable_add(st, "ccc");
    identifier_t *res1;
    identifier_t *res2;
    
    symqueue_add(symqueue, item1);
    symqueue_add(symqueue, item2);
    symqueue_add(symqueue, item3);
    
    res1 = symqueue_rear(symqueue);
    res2 = symqueue_peek(symqueue);
    
    TEST_ASSERT_NOT_NULL(res1);
    TEST_ASSERT_NOT_NULL(res2);
    TEST_ASSERT_EQUAL_PTR(item1, res2);
    TEST_ASSERT_EQUAL_PTR(item3, res1);
}

void test_symqueue_pop_empty(void)
{
    symqueue_t *symqueue = symqueue_create();
    identifier_t *res;

    res = symqueue_pop(symqueue);

    TEST_ASSERT_NULL(res);
}

void test_symqueue_pop(void)
{
    symqueue_t *symqueue = symqueue_create();
    symtable_t *st = symtable_create();
    
    identifier_t *item = symtable_add(st, "aaa");
    symqueue_add(symqueue, item);

    identifier_t *res;
    res = symqueue_pop(symqueue);

    TEST_ASSERT_EQUAL_PTR(item, res);
}

void test_symqueue_is_empty(void)
{
    symqueue_t *symqueue = symqueue_create();
    bool res;
    
    res = symqueue_is_empty(symqueue);
    
    TEST_ASSERT(res);
}

void test_symqueue_isnt_empty(void)
{
    symqueue_t *symqueue = symqueue_create();
    symtable_t *st = symtable_create();
    bool res;
    
    identifier_t *item = symtable_add(st, "aaa");
    symqueue_add(symqueue, item);
    res = symqueue_is_empty(symqueue);
    
    TEST_ASSERT(!res);
}

void test_symqueue_destroy(void)
{
    symqueue_t *symqueue = symqueue_create();
    symqueue_destroy(symqueue);
    
    TEST_PASS();
}

void test_symqueue_rear(void)
{
    symqueue_t *symqueue = symqueue_create();
    symtable_t *st = symtable_create();
    identifier_t *item1 = symtable_add(st, "aaa");
    identifier_t *item2 = symtable_add(st, "bbb");
    identifier_t *res;
    
    symqueue_add(symqueue, item1);
    symqueue_add(symqueue, item2);
    res = symqueue_rear(symqueue);
    
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_PTR(item2, res);
}

void test_symqueue_peek(void)
{
    symqueue_t *symqueue = symqueue_create();
    symtable_t *st = symtable_create();
    
    identifier_t *item1 = symtable_add(st, "aaa");
    identifier_t *item2 = symtable_add(st, "bbb");
    identifier_t *res;
    
    symqueue_add(symqueue, item1);
    symqueue_add(symqueue, item2);
    res = symqueue_peek(symqueue);
    
    TEST_ASSERT_NOT_NULL(res);
    TEST_ASSERT_EQUAL_PTR(item1, res);
}
