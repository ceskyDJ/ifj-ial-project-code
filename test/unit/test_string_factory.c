#include "../../unity/src/unity.h"
#include "../../src/string_factory.h"

#include <string.h>

int append_to_string(string_t *string, char *to_append)
{
    int len = strlen(to_append);
    int ret;

    for (int i = 0; i < len; i++)
        ret = string_appendc(string, to_append[i]);

    return ret;
}

void test_string_create()
{
    string_t *string = string_create();

    TEST_ASSERT_NOT_NULL(string);
    TEST_ASSERT_NOT_NULL(string->content);
}

void test_string_destroy_string()
{
    string_t *string = string_create();

    string_destroy(string);

    TEST_PASS();
}

void test_append_single_character_to_string()
{
    int ret;
    string_t *string = string_create();

    ret = string_appendc(string, 'a');

    TEST_ASSERT_EQUAL_INT(0, ret);
}

void test_append_null_byte_to_string()
{
    int ret;
    string_t *string = string_create();

    ret = string_appendc(string, '\0');

    TEST_ASSERT_EQUAL_INT(0, ret);
}

void test_append_multiple_character_to_string()
{
    char *chars = "aaaaaaaaaaaaaaaaaaaa";
    int ret;
    string_t *string = string_create();

    ret = append_to_string(string, chars);

    TEST_ASSERT_EQUAL_INT(0, ret);
    TEST_ASSERT_EQUAL_INT(strlen(chars), string->len);
    TEST_ASSERT_EQUAL_INT(2*INIT_MAX, string->max_len);
}

void test_string_cmp_empty_str1_empty_str2()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();

    ret = string_cmp(str1, str2);

    TEST_ASSERT_EQUAL_INT(0, ret);
}

void test_string_cmp_same_str1_and_str2()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();
    append_to_string(str1, "hello");
    append_to_string(str2, "hello");

    ret = string_cmp(str1, str2);

    TEST_ASSERT_EQUAL_INT(0, ret);
}

void test_string_cmp_str1_is_different_to_str2()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();

    append_to_string(str1, "hello");
    append_to_string(str2, "helLo");

    ret = string_cmp(str1, str2);

    TEST_ASSERT_NOT_EQUAL_INT(0, ret);
}

void test_string_cmp_str1_is_substring_of_str2()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();
    append_to_string(str1, "hell");
    append_to_string(str2, "helloo");

    ret = string_cmp(str1, str2);

    TEST_ASSERT_NOT_EQUAL_INT(0, ret);
}

void test_string_cmp_str2_is_substring_of_str1()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();
    append_to_string(str1, "helloo");
    append_to_string(str2, "hell");

    ret = string_cmp(str1, str2);

    TEST_ASSERT_NOT_EQUAL_INT(0, ret);
}

void test_string_check_prefix_str1_is_prefix()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();
    append_to_string(str1, "hell");
    append_to_string(str2, "helloo");

    ret = string_check_prefix(str1, str2);

    TEST_ASSERT_EQUAL_INT(0, ret);
}

void test_string_check_prefix_same_strings()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();
    append_to_string(str1, "hell");
    append_to_string(str2, "hell");

    ret = string_check_prefix(str1, str2);

    TEST_ASSERT_EQUAL_INT(0, ret);
}

void test_string_check_prefix_str1_is_not_prefix()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();
    append_to_string(str1, "hello");
    append_to_string(str2, "hell");

    ret = string_check_prefix(str1, str2);

    TEST_ASSERT_NOT_EQUAL_INT(0, ret);
}

void test_string_check_prefix_str1_is_empty()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();
    append_to_string(str1, "");
    append_to_string(str2, "hell");

    ret = string_check_prefix(str1, str2);

    TEST_ASSERT_EQUAL_INT(0, ret);
}

void test_string_check_prefix_str2_is_empty()
{
    int ret;
    string_t *str1 = string_create();
    string_t *str2 = string_create();
    append_to_string(str1, "hell");
    append_to_string(str2, "");

    ret = string_check_prefix(str1, str2);

    TEST_ASSERT_NOT_EQUAL_INT(0, ret);
}

void test_string_export_empty_string()
{
    char *export;
    string_t *str = string_create();

    export = string_export(str);

    TEST_ASSERT_NOT_NULL(export);
    TEST_ASSERT_EQUAL_INT(0, strlen(export));
    TEST_ASSERT_EQUAL_STRING(export, "");
}

void test_string_export_not_empty_string()
{
    char *export;
    string_t *str = string_create();
    append_to_string(str, "hello");

    export = string_export(str);

    TEST_ASSERT_NOT_NULL(export);
    TEST_ASSERT_EQUAL_INT(strlen("hello"), strlen(export));
    TEST_ASSERT_EQUAL_STRING(export, "hello");
}

void test_string_expose()
{
    char *exposed;
    string_t *str = string_create();
    append_to_string(str, "hello");

    exposed = string_expose(str);

    TEST_ASSERT_NOT_NULL(exposed);
    TEST_ASSERT_EQUAL_INT(strlen("hello"), strlen(exposed));
    TEST_ASSERT_EQUAL_STRING(exposed, "hello");
}

