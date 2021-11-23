#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "unity.h"
#include <c_utils/array.h>
#include <c_utils/min.h>

// Unity boilerplate
void setUp(void){}
void tearDown(void){}

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

void test_array_max(void)
{
    int values[] = {1,2,3,4,-9};
    TEST_ASSERT_EQUAL(4, array_max(values, array_length(values)));

    int values2[] = {1,2,3,4,-9, 100};
    TEST_ASSERT_EQUAL(100, array_max(values2, array_length(values2)));

    float floats[] = {-1e9, -1e5, -1e12, -1e9};
    TEST_ASSERT_EQUAL_FLOAT(-1e5, array_max(floats, array_length(floats)));

    float floats2[] = {-1e9, -1e5, 1e12, -1e9};
    float *ptr = floats2;
    TEST_ASSERT_EQUAL_FLOAT(1e12, array_max(ptr, 4));
}

void test_array_min(void)
{
    int values[] = {1,2,3,4,-9};
    TEST_ASSERT_EQUAL(-9, array_min(values, array_length(values)));

    int values2[] = {1,2,3,4,-9, 100};
    TEST_ASSERT_EQUAL(-9, array_min(values2, array_length(values2)));

    float floats[] = {-1e9, -1e5, -1e12, -1e9};
    TEST_ASSERT_EQUAL_FLOAT(-1e12, array_min(floats, array_length(floats)));

    float floats2[] = {-1e9, -1e5, 1e12, -1e9};
    float *ptr = floats2;
    TEST_ASSERT_EQUAL_FLOAT(-1e9, array_min(ptr, 4));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_is_array);
    RUN_TEST(test_array_length);
    RUN_TEST(test_array_max);
    RUN_TEST(test_array_min);

    UNITY_END();

    return 0;
}
