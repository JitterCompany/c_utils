#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "unity.h"
#include "round.h"

void test_divide_round_up_4_div_1(void)
{
    TEST_ASSERT_EQUAL(4, divide_round_up(4,1));
}
void test_divide_round_up_0_div_1(void)
{
    TEST_ASSERT_EQUAL(1, divide_round_up(0,1));
}
void test_divide_round_up_3_div_2(void)
{
    TEST_ASSERT_EQUAL(2, divide_round_up(3,2));
}
void test_divide_round_up_neg3_div_2(void)
{
    TEST_ASSERT_EQUAL(-1, divide_round_up(-3,2));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_divide_round_up_4_div_1);
    RUN_TEST(test_divide_round_up_0_div_1);
    RUN_TEST(test_divide_round_up_3_div_2);
    RUN_TEST(test_divide_round_up_neg3_div_2);

    UNITY_END();

    return 0;
}
