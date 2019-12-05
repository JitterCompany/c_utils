#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "unity.h"
#include <c_utils/array.h>
#include <c_utils/min.h>

void _test_is_array_helper(int pseudo_a[], int pseudo_b[2])
{
    // This is a 'real' array
    int real_arr[2];
    TEST_ASSERT_EQUAL(true, IS_ARRAY(real_arr));

    // Arrays passed as args are not actually arrays. They are just syntax
    // sugar around pointers!!
    TEST_ASSERT_EQUAL(false, IS_ARRAY(pseudo_a));
    TEST_ASSERT_EQUAL(false, IS_ARRAY(pseudo_b));

}

void test_is_array(void)
{
    uint8_t arr[1];
    uint8_t *ptr = arr;
    TEST_ASSERT_EQUAL(true, IS_ARRAY(arr));
    TEST_ASSERT_EQUAL(false, IS_ARRAY(ptr));

    int real_arr[2] = {3, 4};
    _test_is_array_helper(real_arr, real_arr);

}

void test_array_length(void)
{
    uint8_t bytes[4];
    int32_t ints[5];
    TEST_ASSERT_EQUAL(4, array_length(bytes));
    TEST_ASSERT_EQUAL(5, array_length(ints));

    // This should compile!
    min(0U, array_length(ints));

    //int32_t *ptr = ints;
    //TEST_ASSERT_EQUAL(5, array_length(ptr)); // should trigger a STATIC_ASSERT() fail
}


int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_is_array);
    RUN_TEST(test_array_length);

    UNITY_END();

    return 0;
}
