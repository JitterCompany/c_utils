#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "str.h"
#include "unity.h"
#include <stdlib.h>
#if defined(__linux__)
#include <bsd/string.h>
#endif

// Unity boilerplate
void setUp(void){}
void tearDown(void){}

void test_str_toupper(void)
{
    const char *reference = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const char *input = "abcdefghijklmnopqrstuvwxyz0123456789";
    char buffer[strlen(input)+1];
    strlcpy(buffer, input, sizeof(buffer));

    str_toupper(buffer);
    TEST_ASSERT_EQUAL_MEMORY(buffer, reference, sizeof(buffer));
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_str_toupper);
    UNITY_END();



    return 0;
}
