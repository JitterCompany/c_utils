#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "long_long_to_str.h"
#include "unity.h"
#include <stdlib.h>


void test__hardcoded__match(void)
{
    char string[256];

    
    // Positive
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), 314));
    TEST_ASSERT_EQUAL_STRING("314",  string);

    // Large Positive
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), 99999999));
    TEST_ASSERT_EQUAL_STRING("99999999",  string);


    // Zero
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), 0));
    TEST_ASSERT_EQUAL_STRING("0",  string);


    // Negative
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), -1));
    TEST_ASSERT_EQUAL_STRING("-1",  string);


    // Large Negative
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), -99999999));
    TEST_ASSERT_EQUAL_STRING("-99999999",  string);


    // Near INT64_MAX
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), INT64_MAX-1));
    TEST_ASSERT_EQUAL_STRING("9223372036854775806",  string);

    // INT64_MAX
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), INT64_MAX));
    TEST_ASSERT_EQUAL_STRING("9223372036854775807",  string);

    // Near INT64_MIN
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), INT64_MIN+1));
    TEST_ASSERT_EQUAL_STRING("-9223372036854775807",  string);

    // INT64_MIN
    memset(string, 0x33, 256);
    string[255] = 0;

    TEST_ASSERT_TRUE(long_long_to_str(string, sizeof(string), INT64_MIN));
    TEST_ASSERT_EQUAL_STRING("-9223372036854775808",  string);
}



int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test__hardcoded__match);
    UNITY_END();



    return 0;
}
