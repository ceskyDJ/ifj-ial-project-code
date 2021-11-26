#include "../../unity/src/unity.h"
#include "../../src/scanner.h"

void test_load_token_one(void)
{
#undef getchar
#define getchar fgetc(stdin)
    
}
