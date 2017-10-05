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
    uint8_t data[1024];
    Ringbuffer ring;
    TEST_ASSERT_FALSE(ringbuffer_is_initialized(&ring));
    ringbuffer_init(&ring, data, 1, sizeof(data));
    TEST_ASSERT(ringbuffer_is_initialized(&ring));
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init);

    UNITY_END();

    return 0;
}
