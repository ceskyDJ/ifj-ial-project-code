#include "../../unity/src/unity.h"
#include "../../src/kwtable.h"

void test_kwtable_create_success(void) {
    kwtable_create();

    TEST_PASS();
}

void test_kwtable_create_result(void) {
    kwtable_t *table = kwtable_create();

    TEST_ASSERT_NOT_NULL(table);
}

void test_kwtable_destroy_success(void) {
    kwtable_t *table = kwtable_create();

    kwtable_destroy(table);

    TEST_PASS();
}

void test_kwtable_find_non_existing_keyword(void) {
    kwtable_t *table = kwtable_create();

    keyword_t *keyword = kwtable_find(table, "non-existing");

    TEST_ASSERT_NULL(keyword);
}

void test_kwtable_find_existing_keyword_first(void) {
    kwtable_t *table = kwtable_create();

    keyword_t *keyword = kwtable_find(table, "do");

    TEST_ASSERT_NOT_NULL(keyword);
    TEST_ASSERT_EQUAL_INT(KW_DO, *keyword);
}

void test_kwtable_find_existing_keyword_last(void) {
    kwtable_t *table = kwtable_create();

    keyword_t *keyword = kwtable_find(table, "while");

    TEST_ASSERT_NOT_NULL(keyword);
    TEST_ASSERT_EQUAL_INT(KW_WHILE, *keyword);
}

void test_kwtable_find_more_keywords(void) {
    kwtable_t *table = kwtable_create();

    keyword_t *keyword_1 = kwtable_find(table, "if");
    keyword_t *keyword_2 = kwtable_find(table, "elif");
    keyword_t *keyword_3 = kwtable_find(table, "else");

    TEST_ASSERT_NOT_NULL(keyword_1);
    TEST_ASSERT_EQUAL_INT(KW_IF, *keyword_1);
    TEST_ASSERT_NULL(keyword_2);
    TEST_ASSERT_NOT_NULL(keyword_3);
    TEST_ASSERT_EQUAL_INT(KW_ELSE, *keyword_3);
}
