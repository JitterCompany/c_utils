#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "unity.h"
#include "ringbuffer.h"

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
    TEST_ASSERT_EQUAL(ringbuffer_used_count(&ring), 0);

    TEST_ASSERT_FALSE(ringbuffer_is_full(&ring));
    TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&ring));

    TEST_ASSERT_EQUAL(ringbuffer_get_element_size(&ring), 3);
    TEST_ASSERT_EQUAL(ringbuffer_free_count(&ring), 17);

    // not tested yet:
    //
    // write
    // read
    // flush
    //
    // get_writeable
    // commit
    // get_readable
    // get_readable_offset
    // advance
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init);

    UNITY_END();

    return 0;
}
