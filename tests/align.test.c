#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "unity.h"
#include "align.h"

// Unity boilerplate
void setUp(void){}
void tearDown(void){}

void test_align_4_mod_0(void)
{
    uint32_t value;
    void *unaligned = &value;
    void *aligned = align(unaligned, 4);

    TEST_ASSERT_EQUAL(0, ((uintptr_t)unaligned) % 4);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 4);
    TEST_ASSERT_EQUAL_PTR(unaligned, aligned);
}
void test_align_4_mod_1(void)
{
    uint32_t value;
    void *unaligned = ((uint8_t*)&value -3);
    void *aligned = align(unaligned, 4);

    TEST_ASSERT_EQUAL(1, ((uintptr_t)unaligned) % 4);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 4);
    TEST_ASSERT_EQUAL_PTR((void*)&value, aligned);
}
void test_align_4_mod_2(void)
{
    uint32_t value;
    void *unaligned = ((uint8_t*)&value -2);
    void *aligned = align(unaligned, 4);

    TEST_ASSERT_EQUAL(2, ((uintptr_t)unaligned) % 4);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 4);
    TEST_ASSERT_EQUAL_PTR((void*)&value, aligned);
}
void test_align_4_mod_3(void)
{
    uint32_t value;
    void *unaligned = ((uint8_t*)&value -1);
    void *aligned = align(unaligned, 4);

    TEST_ASSERT_EQUAL(3, ((uintptr_t)unaligned) % 4);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 4);
    TEST_ASSERT_EQUAL_PTR((void*)&value, aligned);
}

void test_align_8_mod_3(void)
{
    uint64_t value;
    void *unaligned = ((uint8_t*)&value -3);
    void *aligned = align(unaligned, 8);

    TEST_ASSERT_EQUAL(5, ((uintptr_t)unaligned) % 8);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 8);
    TEST_ASSERT_EQUAL_PTR((void*)&value, aligned);
}

void test_align_16(void)
{
    uint64_t value;
    void *unaligned = ((uint8_t*)&value -7);
    void *aligned = align(unaligned, 16);

    TEST_ASSERT_NOT_EQUAL(0, ((uintptr_t)unaligned) % 16);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 16);
    TEST_ASSERT(aligned < (void*)(((uint8_t*)&value)+16));
}
void test_align_32(void)
{
    uint64_t value;
    void *unaligned = ((uint8_t*)&value -7);
    void *aligned = align(unaligned, 32);

    TEST_ASSERT_NOT_EQUAL(0, ((uintptr_t)unaligned) % 32);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 32);
    TEST_ASSERT(aligned < (void*)(((uint8_t*)&value)+32));
}
void test_align_64(void)
{
    uint64_t value;
    void *unaligned = ((uint8_t*)&value -7);
    void *aligned = align(unaligned, 64);

    TEST_ASSERT_NOT_EQUAL(0, ((uintptr_t)unaligned) % 64);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 64);
    TEST_ASSERT(aligned < (void*)(((uint8_t*)&value)+64));
}
void test_align_128(void)
{
    uint64_t value;
    void *unaligned = ((uint8_t*)&value -7);
    void *aligned = align(unaligned, 128);

    TEST_ASSERT_NOT_EQUAL(0, ((uintptr_t)unaligned) % 128);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 128);
    TEST_ASSERT(aligned < (void*)(((uint8_t*)&value)+128));
}
void test_align_256(void)
{
    uint64_t value;
    void *unaligned = ((uint8_t*)&value -7);
    void *aligned = align(unaligned, 256);

    TEST_ASSERT_NOT_EQUAL(0, ((uintptr_t)unaligned) % 256);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 256);
    TEST_ASSERT(aligned < (void*)(((uint8_t*)&value)+256));
}
void test_align_512(void)
{
    uint64_t value;
    void *unaligned = ((uint8_t*)&value -7);
    void *aligned = align(unaligned, 512);

    TEST_ASSERT_NOT_EQUAL(0, ((uintptr_t)unaligned) % 512);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 512);
    TEST_ASSERT(aligned < (void*)(((uint8_t*)&value)+512));
}

void test_align_32_mod_3(void)
{
    uint64_t value;
    void *unaligned = ((uint8_t*)&value -3);
    void *aligned = align(unaligned, 32);

    TEST_ASSERT_NOT_EQUAL(0, ((uintptr_t)unaligned) % 32);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 32);
    TEST_ASSERT(aligned < (void*)(((uint8_t*)&value)+32));
}

void test_align_4_mod_3_wrapper(void)
{
    uint32_t value;
    void *unaligned = ((uint8_t*)&value -1);
    void *aligned = align_4(unaligned);

    TEST_ASSERT_EQUAL(3, ((uintptr_t)unaligned) % 4);
    TEST_ASSERT_EQUAL(0, ((uintptr_t)aligned) % 4);
    TEST_ASSERT_EQUAL_PTR((void*)&value, aligned);
}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_align_4_mod_0);
    RUN_TEST(test_align_4_mod_1);
    RUN_TEST(test_align_4_mod_2);
    RUN_TEST(test_align_4_mod_3);

    RUN_TEST(test_align_8_mod_3);
    RUN_TEST(test_align_16);
    RUN_TEST(test_align_32);
    RUN_TEST(test_align_64);
    RUN_TEST(test_align_128);
    RUN_TEST(test_align_256);
    RUN_TEST(test_align_512);

    RUN_TEST(test_align_4_mod_3_wrapper);

    UNITY_END();

    return 0;
}
