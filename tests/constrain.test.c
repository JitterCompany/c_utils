#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "unity.h"
#include "constrain.h"

void test_constrain_below(void)
{
    TEST_ASSERT_EQUAL(-8, constrain(-100,  -8, 19));
    TEST_ASSERT_EQUAL(-8, constrain(-9,  -8, 19));
    TEST_ASSERT_EQUAL(-8, constrain(-8,  -8, 19));
}

void test_constrain_within(void)
{
    TEST_ASSERT_EQUAL(-7, constrain(-7,  -8, 19));
    TEST_ASSERT_EQUAL(-1, constrain(-1,  -8, 19));
    TEST_ASSERT_EQUAL(0, constrain(0,  -8, 19));
    TEST_ASSERT_EQUAL(1, constrain(1,  -8, 19));
    TEST_ASSERT_EQUAL(18, constrain(18,  -8, 19));
}

void test_constrain_above(void)
{
    TEST_ASSERT_EQUAL(19, constrain(19,  -8, 19));
    TEST_ASSERT_EQUAL(19, constrain(20,  -8, 19));
    TEST_ASSERT_EQUAL(19, constrain(100,  -8, 19));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_constrain_below);
    RUN_TEST(test_constrain_within);
    RUN_TEST(test_constrain_above);

    UNITY_END();

    return 0;
}
