#include "../../unity/src/unity.h"
#include "../../src/stack.h"

void test_stack_init(void){
    stack_t stack;
    stack_init(stack);
    TEST_ASSERT_NOT_NULL(stack);
}

void test_stack_dispose(void){
    stack_t stack;
    stack_init(stack);
    stack_dispose(s);
    TEST_PASS();
}

void test_stack_push(void){
    stack_t stack;
    stack_init(stack);
    symtable_t *t = symtable_create();
    stack_push(stack, t);
    TEST_ASSERT(stack.top != NULL);
    TEST_ASSERT(stack.size == 1);
}

void test_stack_pop(void){
    stack_t stack;
    stack_init(stack);
    TEST_ASSERT_NOT_NULL(stack);
}

void test_first_active(void){
    stack_t stack;
    stack_init(stack);
    symtable_t *t = symtable_create();
    stack_push(stack, t);
    first_active(stack);
    TEST_ASSERT(stack.act == stack.top);
}

void test_next_active(void){
    stack_t stack;
    stack_init(stack);
    symtable_t *t = symtable_create();
    symtable_t *t2 = symtable_create();
    stack_push(stack, t);
    stack_push(stack, t2);
    first_active(stack);
    next_active(stack);
    TEST_ASSERT(stack.act == *t2);
}
