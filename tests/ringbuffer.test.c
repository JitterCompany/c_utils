#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "unity.h"
#include "ringbuffer.h"

// Unity boilerplate
void setUp(void){}
void tearDown(void){}

void assert(bool sane)
{
    TEST_ASSERT_MESSAGE(sane, "Assertion failed!");
}

void test_init(void)
{
    uint8_t data[3*17];
    Ringbuffer ring;
    memset(&ring, 0xFF, sizeof(ring));

    TEST_ASSERT_FALSE(ringbuffer_is_initialized(&ring));
    ringbuffer_init(&ring, data, 3, 17);
    TEST_ASSERT(ringbuffer_is_initialized(&ring));

    TEST_ASSERT(ringbuffer_is_empty(&ring));
    TEST_ASSERT_EQUAL(0, ringbuffer_used_count(&ring));

    TEST_ASSERT_FALSE(ringbuffer_is_full(&ring));
    TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&ring));

    TEST_ASSERT_EQUAL(3, ringbuffer_get_element_size(&ring));
    TEST_ASSERT_EQUAL(17, ringbuffer_free_count(&ring));
}

void test_init_empty(void)
{
    Ringbuffer ring;
    memset(&ring, 0xFF, sizeof(ring));

    TEST_ASSERT_FALSE(ringbuffer_is_initialized(&ring));
    ringbuffer_init(&ring, NULL, 0, 0);
    TEST_ASSERT(ringbuffer_is_initialized(&ring));

    TEST_ASSERT(ringbuffer_is_empty(&ring));
    TEST_ASSERT_EQUAL(0, ringbuffer_used_count(&ring));

    TEST_ASSERT_TRUE(ringbuffer_is_full(&ring));
    TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&ring));

    TEST_ASSERT_EQUAL(0, ringbuffer_get_element_size(&ring));
    TEST_ASSERT_EQUAL(0, ringbuffer_free_count(&ring));

    TEST_ASSERT_EQUAL(0, ringbuffer_get_readable(&ring));
    TEST_ASSERT_EQUAL(0, ringbuffer_get_writeable(&ring));
}

void test_write(void)
{
    uint8_t data[5*2];
    Ringbuffer ring;
    ringbuffer_init(&ring, data, 5, 2);

    // first write should succeed
    TEST_ASSERT_EQUAL(2, ringbuffer_free_count(&ring));
    TEST_ASSERT_EQUAL(1, ringbuffer_write(&ring, "TEST", 1));
    TEST_ASSERT_FALSE(ringbuffer_is_full(&ring));
    TEST_ASSERT_EQUAL(1, ringbuffer_free_count(&ring));
    TEST_ASSERT_EQUAL(1, ringbuffer_used_count(&ring));
    TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&ring));

    // second write should succeed as well, but the ringbuffer is now full
    TEST_ASSERT_EQUAL(1, ringbuffer_write(&ring, "ABCD", 1));
    TEST_ASSERT_TRUE(ringbuffer_is_full(&ring));
    TEST_ASSERT_EQUAL(0, ringbuffer_free_count(&ring));
    TEST_ASSERT_EQUAL(2, ringbuffer_used_count(&ring));
    TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&ring));

    // third write should fail, ringbuffer is now overflowed
    TEST_ASSERT_EQUAL(0, ringbuffer_write(&ring, "EOF!", 1));
    TEST_ASSERT_TRUE(ringbuffer_is_full(&ring));
    TEST_ASSERT_EQUAL(0, ringbuffer_free_count(&ring));
    TEST_ASSERT_EQUAL(2, ringbuffer_used_count(&ring));
    TEST_ASSERT_TRUE(ringbuffer_is_overflowed(&ring));
}

void test_read(void)
{
    uint8_t data[5*2];
    Ringbuffer ring;
    ringbuffer_init(&ring, data, 5, 2);

    TEST_ASSERT_EQUAL(1, ringbuffer_write(&ring, "TEST", 1));
    TEST_ASSERT_FALSE(ringbuffer_is_empty(&ring));
    TEST_ASSERT_EQUAL(1, ringbuffer_used_count(&ring));

    // first read: should read back the test string
    char result[5];
    TEST_ASSERT_EQUAL(1, ringbuffer_read(&ring, result, 1));
    TEST_ASSERT_EQUAL_STRING("TEST", result);
    TEST_ASSERT_TRUE(ringbuffer_is_empty(&ring));
    TEST_ASSERT_EQUAL(0, ringbuffer_used_count(&ring));

    // second read: should fail because the ringbuffer is empty
    strcpy(result, "ABCD");
    TEST_ASSERT_EQUAL(0, ringbuffer_read(&ring, result, 1));
    TEST_ASSERT_EQUAL_STRING("ABCD", result);
}

void test_write_multiple_flush(void)
{
    uint8_t data[5*3];
    Ringbuffer ring;
    ringbuffer_init(&ring, data, 5, 3);

    // first two writes should succeed
    TEST_ASSERT_EQUAL(2, ringbuffer_write(&ring, "TEST\0ABCD\0", 2));
    TEST_ASSERT_EQUAL(2, ringbuffer_used_count(&ring));

    // only the first of these writes succeeds: ringbuffer full!
    TEST_ASSERT_EQUAL(1, ringbuffer_write(&ring, "EFGH\0IJKL\0", 2));
    TEST_ASSERT_EQUAL(3, ringbuffer_used_count(&ring));
    TEST_ASSERT(ringbuffer_is_full(&ring));

    // flush first two items (TEST, ABCD)
    ringbuffer_flush(&ring, 2);

    // third item should be read out last
    char result[5];
    TEST_ASSERT_EQUAL(1, ringbuffer_read(&ring, result, 1));
    TEST_ASSERT_EQUAL_STRING("EFGH", result);
    TEST_ASSERT_TRUE(ringbuffer_is_empty(&ring));
    TEST_ASSERT_EQUAL(0, ringbuffer_used_count(&ring));
}

void test_wraparound(void)
{
    uint8_t data[5*3];
    Ringbuffer ring;
    ringbuffer_init(&ring, data, 5, 3);
    for(size_t n=0;n<10;n++) {
        TEST_ASSERT_EQUAL(NULL, ringbuffer_get_readable_offset(&ring, n));
    }

    // write two elements
    TEST_ASSERT_EQUAL(2, ringbuffer_write(&ring, "TEST\0ABCD\0", 2));

    // read_ptr at 0, write_ptr at 2, expect data at read offset 0, 1
    char *element = ringbuffer_get_readable_offset(&ring, 0);
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL_STRING("TEST", element);

    element = ringbuffer_get_readable_offset(&ring, 1);
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL_STRING("ABCD", element);

    TEST_ASSERT_EQUAL(NULL, ringbuffer_get_readable_offset(&ring, 2));
    ringbuffer_flush(&ring, 2);

    // ringbuffer is empty again, write three more elements
    TEST_ASSERT_EQUAL(3, ringbuffer_write(&ring, "EFGH\0IJKL\0MNOP", 3));
    TEST_ASSERT(ringbuffer_is_full(&ring));

    // read_ptr at 2, write_ptr at 2 (wrapped): expect data at read offset 0, 1, 2
    element = ringbuffer_get_readable_offset(&ring, 0);
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL_STRING("EFGH", element);

    element = ringbuffer_get_readable_offset(&ring, 1);
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL_STRING("IJKL", element);

    element = ringbuffer_get_readable_offset(&ring, 2);
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL_STRING("MNOP", element);

    TEST_ASSERT_EQUAL(NULL, ringbuffer_get_readable_offset(&ring, 3));
    ringbuffer_flush(&ring, 1);

    // read_ptr at 0, write_ptr at 2: expect data at read offset 0, 1
    element = ringbuffer_get_readable_offset(&ring, 0);
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL_STRING("IJKL", element);

    element = ringbuffer_get_readable_offset(&ring, 1);
    TEST_ASSERT_NOT_NULL(element);
    TEST_ASSERT_EQUAL_STRING("MNOP", element);

    TEST_ASSERT_EQUAL(NULL, ringbuffer_get_readable_offset(&ring, 2));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init);
    RUN_TEST(test_init_empty);
    RUN_TEST(test_write);
    RUN_TEST(test_read);
    RUN_TEST(test_write_multiple_flush);
    RUN_TEST(test_wraparound);

    UNITY_END();

    return 0;
}
