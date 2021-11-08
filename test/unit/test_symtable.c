#include "../../unity/src/unity.h"
#include "../../src/symtable.h"

void test_create()
{
    symtable_t *t1 = symtable_create();
    TEST_MESSAGE("created t1\n");
    symtable_t *t2 = symtable_create();

    TEST_ASSERT_NOT_NULL(t1);
    TEST_ASSERT_NOT_NULL(t2);
}

void test_destroy_table()
{
    symtable_t *t = symtable_create();

    symtable_destroy(t);

    TEST_PASS();
}

void test_add_name_to_table()
{
    symtable_t *t = symtable_create();

    identifier_t *id1 = symtable_add(t, "foo");
    identifier_t *id2 = symtable_add(t, "foooooooooooooooooooooooooooooo");
    identifier_t *id3 = symtable_add(t, "");

    TEST_ASSERT_NOT_NULL(id1);
    TEST_ASSERT_NOT_NULL(id2);
    TEST_ASSERT_NOT_NULL(id3);
    TEST_ASSERT_EQUAL_STRING(id1->name, "foo");
    TEST_ASSERT_EQUAL_STRING(id2->name, "foooooooooooooooooooooooooooooo");
    TEST_ASSERT_EQUAL_STRING(id3->name, "");
}

void test_add_same_name_multiple_times_to_table()
{
    symtable_t *t = symtable_create();

    identifier_t *id1 = symtable_add(t, "foo");
    identifier_t *id2 = symtable_add(t, "foo");
    identifier_t *id3 = symtable_add(t, "foo");

    TEST_ASSERT_NOT_NULL(id1);
    TEST_ASSERT_NOT_NULL(id2);
    TEST_ASSERT_NOT_NULL(id3);
    TEST_ASSERT_EQUAL(id1, id2);
    TEST_ASSERT_EQUAL(id1, id3);
    TEST_ASSERT_EQUAL(id2, id3);
    TEST_ASSERT_EQUAL_STRING(id1->name, "foo");
}

void test_find_name_in_empty_table()
{
    symtable_t *t = symtable_create();

    identifier_t *id = symtable_find(t, "foo");

    TEST_ASSERT_NULL(id);
}

void test_find_name_in_table_containing_different_names()
{
    symtable_t *t = symtable_create();
    symtable_add(t, "bar");
    symtable_add(t, "arb");
    symtable_add(t, "rba");

    identifier_t *id = symtable_find(t, "foo");

    TEST_ASSERT_NULL(id);
}

void test_find_name_in_table_containing_name()
{
    symtable_t *t = symtable_create();
    symtable_add(t, "bar");
    symtable_add(t, "arb");
    symtable_add(t, "rba");
    symtable_add(t, "foo");

    identifier_t *id = symtable_find(t, "foo");

    TEST_ASSERT_NOT_NULL(id);
    TEST_ASSERT_EQUAL_STRING(id->name, "foo");
}

void test_check_new_identifier_properties_are_zero()
{
    symtable_t *t = symtable_create();

    identifier_t *id = symtable_add(t, "foo");

    TEST_ASSERT_EQUAL_INT(0, id->line);
    TEST_ASSERT_EQUAL_INT(0, id->character);
    TEST_ASSERT_EQUAL_INT(0, id->type);
    TEST_ASSERT_EQUAL_INT(0, id->var.type);
    TEST_ASSERT_EQUAL_INT(0, id->var.init);
    TEST_ASSERT_EQUAL_INT(0, id->var.used);
    TEST_ASSERT_EQUAL_INT(0, id->fun.defined);
    TEST_ASSERT_NULL(id->fun.param);
    TEST_ASSERT_NULL(id->fun.retval);
}

void test_check_retrieved_variable_identifier_properties()
{
    int line = 1;
    int character = 1;
    int init = 1;
    int used = 1;
    symtable_t *t = symtable_create();
    identifier_t *id = symtable_add(t, "foo");

    id->line = line;
    id->character = character;
    id->type = VARIABLE;
    id->var.type = STRING;
    id->var.init = init;
    id->var.used = used;
    identifier_t *check = symtable_find(t, "foo");

    TEST_ASSERT_EQUAL_INT(check->line, line);
    TEST_ASSERT_EQUAL_INT(check->character, character);
    TEST_ASSERT_EQUAL_INT(check->type, VARIABLE);
    TEST_ASSERT_EQUAL_INT(check->var.type, STRING);
    TEST_ASSERT_EQUAL_INT(check->var.init, init);
    TEST_ASSERT_EQUAL_INT(check->var.used, used);
}

void test_check_retrieved_function_identifier_properties()
{
    int line = 1;
    int character = 1;
    int defined = 1;
    char *param = "ssis";
    char *retval = "";
    symtable_t *t = symtable_create();
    identifier_t *id = symtable_add(t, "foo");

    id->line = line;
    id->character = character;
    id->type = FUNCTION;
    id->fun.defined = defined;
    id->fun.param = param;
    id->fun.retval = retval;

    identifier_t *check = symtable_find(t, "foo");

    TEST_ASSERT_EQUAL_INT(check->line, line);
    TEST_ASSERT_EQUAL_INT(check->character, character);
    TEST_ASSERT_EQUAL_INT(check->type, FUNCTION);
    TEST_ASSERT_EQUAL_INT(check->fun.defined, defined);
    TEST_ASSERT_NOT_NULL(check->fun.param);
    TEST_ASSERT_NOT_NULL(check->fun.retval);
    TEST_ASSERT_EQUAL_STRING(check->fun.param, param);
    TEST_ASSERT_EQUAL_STRING(check->fun.retval, retval);
}

