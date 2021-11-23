#include <stdbool.h>
#include <string.h>
#include <stddef.h>

#include "unity.h"
#include "retry_ringbuffer.h"

int g_remaining_asserts = 0;

// Unity boilerplate
void setUp(void){}
void tearDown(void){}

void assert(bool sane)
{
    if(g_remaining_asserts) {
        if(!sane) {
            g_remaining_asserts--;
        }
    } else {
        TEST_ASSERT_MESSAGE(sane, "Assertion failed!");
    }
}

void test_init(void)
{
    g_remaining_asserts = 0;

    uint8_t buffer[3*5];
    Ringbuffer rb;
    ringbuffer_init(&rb, buffer, 5, 3);
    RetryRingbuffer la_rb;
    retry_ringbuffer_init(&la_rb, &rb);
}

void test_implemented(void)
{
    g_remaining_asserts = 0;

    uint8_t buffer[3*5];
    Ringbuffer rb;
    ringbuffer_init(&rb, buffer, 5, 3);
    RetryRingbuffer la_rb;
    retry_ringbuffer_init(&la_rb, &rb);

    void *write = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    retry_ringbuffer_cancel_write(&la_rb, write);
    write = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    retry_ringbuffer_complete_write(&la_rb, write);

    void *read = retry_ringbuffer_claim_read_ptr(&la_rb);
    retry_ringbuffer_cancel_all_reads(&la_rb);
    read = retry_ringbuffer_claim_read_ptr(&la_rb);
    retry_ringbuffer_complete_all_reads(&la_rb);
}

void test_full_empty(void)
{
    uint8_t buffer[3*5];
    Ringbuffer rb;
    ringbuffer_init(&rb, buffer, 5, 3);
    RetryRingbuffer la_rb;
    retry_ringbuffer_init(&la_rb, &rb);

    TEST_ASSERT_TRUE(retry_ringbuffer_is_empty(&la_rb));

    retry_ringbuffer_wrapping_write_ptr(&la_rb);
    retry_ringbuffer_complete_write(&la_rb, NULL);
    TEST_ASSERT_FALSE(retry_ringbuffer_is_empty(&la_rb));
    TEST_ASSERT_FALSE(retry_ringbuffer_is_full(&la_rb));

    retry_ringbuffer_wrapping_write_ptr(&la_rb);
    retry_ringbuffer_complete_write(&la_rb, NULL);
    TEST_ASSERT_FALSE(retry_ringbuffer_is_empty(&la_rb));
    TEST_ASSERT_FALSE(retry_ringbuffer_is_full(&la_rb));

    retry_ringbuffer_wrapping_write_ptr(&la_rb);
    retry_ringbuffer_complete_write(&la_rb, NULL);

    TEST_ASSERT_FALSE(retry_ringbuffer_is_empty(&la_rb));
    TEST_ASSERT_TRUE(retry_ringbuffer_is_full(&la_rb));

}

void print_buffer(char * buf, size_t N)
{
    for (size_t i=0; i < N; i++) {
        printf("[%u]: %c\n", i, buf[i]);
    }
}

void test_claim_all(void)
{
    g_remaining_asserts = 0;

    uint8_t buffer[3*1];
    Ringbuffer rb;
    ringbuffer_init(&rb, buffer, 1, 3);
    RetryRingbuffer la_rb;
    retry_ringbuffer_init(&la_rb, &rb);

    TEST_ASSERT_TRUE(retry_ringbuffer_is_empty(&la_rb));

    char *w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'A';
    retry_ringbuffer_complete_write(&la_rb, NULL);
    TEST_ASSERT_FALSE(retry_ringbuffer_is_empty(&la_rb));
    TEST_ASSERT_FALSE(retry_ringbuffer_is_full(&la_rb));

    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'B';
    retry_ringbuffer_complete_write(&la_rb, NULL);
    TEST_ASSERT_FALSE(retry_ringbuffer_is_empty(&la_rb));
    TEST_ASSERT_FALSE(retry_ringbuffer_is_full(&la_rb));

    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'C';
    retry_ringbuffer_complete_write(&la_rb, NULL);

    print_buffer(buffer, 3);

    TEST_ASSERT_FALSE(retry_ringbuffer_is_empty(&la_rb));
    TEST_ASSERT_TRUE(retry_ringbuffer_is_full(&la_rb));

    void *read1 = retry_ringbuffer_claim_read_ptr(&la_rb);
    void *read2 = retry_ringbuffer_claim_read_ptr(&la_rb);
    void *read3 = retry_ringbuffer_claim_read_ptr(&la_rb);
    void *read4 = retry_ringbuffer_claim_read_ptr(&la_rb);
    TEST_ASSERT_TRUE(read3);
    TEST_ASSERT_FALSE(read4);

    char *write1 = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *write1 = 'D';
    retry_ringbuffer_complete_write(&la_rb, NULL);

    print_buffer(buffer, 3);
    retry_ringbuffer_is_full(&la_rb);

    retry_ringbuffer_complete_all_reads(&la_rb);
    char *read5 = retry_ringbuffer_claim_read_ptr(&la_rb);

    TEST_ASSERT_EQUAL_CHAR('D', *read5);
    retry_ringbuffer_complete_all_reads(&la_rb);

    TEST_ASSERT_TRUE(retry_ringbuffer_is_empty(&la_rb));
}


void test_reproduce_bug(void)
{
    size_t element_count = 2;
    size_t element_size = 1;
    uint8_t buffer[element_count*element_size];
    Ringbuffer rb;
    ringbuffer_init(&rb, buffer, element_size, element_count);
    RetryRingbuffer la_rb;
    retry_ringbuffer_init(&la_rb, &rb);

    printf("=> write\n");
    char *w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'A';
    retry_ringbuffer_complete_write(&la_rb, NULL);

    print_buffer(buffer, element_count);

    printf("=> write\n");
    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'B';
    retry_ringbuffer_complete_write(&la_rb, NULL);
    print_buffer(buffer, element_count);

    printf("=> claim read ptr\n");
    retry_ringbuffer_claim_read_ptr(&la_rb);

    // we're omitting this call on purpose: retry_ringbuffer_complete_all_reads(&la_rb);

    print_buffer(buffer, element_count);

    printf("=> write\n");
    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'D';
    retry_ringbuffer_complete_write(&la_rb, NULL);


    retry_ringbuffer_complete_all_reads(&la_rb);
}

void test_write_without_reads()
{
    size_t element_count = 2;
    size_t element_size = 1;
    uint8_t buffer[element_count*element_size];
    Ringbuffer rb;
    ringbuffer_init(&rb, buffer, element_size, element_count);
    RetryRingbuffer la_rb;
    retry_ringbuffer_init(&la_rb, &rb);

    printf("=> write\n");
    char *w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'A';
    retry_ringbuffer_complete_write(&la_rb, NULL);

    print_buffer(buffer, element_count);

    printf("=> write\n");
    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'B';
    retry_ringbuffer_complete_write(&la_rb, NULL);
    print_buffer(buffer, element_count);

    printf("=> write\n");
    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'C';
    retry_ringbuffer_complete_write(&la_rb, NULL);
    print_buffer(buffer, element_count);


    char *rp = retry_ringbuffer_claim_read_ptr(&la_rb);
    TEST_ASSERT_EQUAL_CHAR('B', *rp);
    rp = retry_ringbuffer_claim_read_ptr(&la_rb);
    TEST_ASSERT_EQUAL_CHAR('C', *rp);

}

void test_write_with_dual_overwrites()
{
    size_t element_count = 2;
    size_t element_size = 1;
    uint8_t buffer[element_count*element_size];
    Ringbuffer rb;
    ringbuffer_init(&rb, buffer, element_size, element_count);
    RetryRingbuffer la_rb;
    retry_ringbuffer_init(&la_rb, &rb);

    printf("=> write\n");
    char *w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'A';
    retry_ringbuffer_complete_write(&la_rb, NULL);

    print_buffer(buffer, element_count);

    printf("=> write\n");
    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'B';
    retry_ringbuffer_complete_write(&la_rb, NULL);
    print_buffer(buffer, element_count);

    printf("=> write\n");
    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'C';
    retry_ringbuffer_complete_write(&la_rb, NULL);
    print_buffer(buffer, element_count);

    printf("=> write\n");
    w = retry_ringbuffer_wrapping_write_ptr(&la_rb);
    *w = 'D';
    retry_ringbuffer_complete_write(&la_rb, NULL);
    print_buffer(buffer, element_count);


    char *rp = retry_ringbuffer_claim_read_ptr(&la_rb);
    TEST_ASSERT_EQUAL_CHAR('C', *rp);
    rp = retry_ringbuffer_claim_read_ptr(&la_rb);
    TEST_ASSERT_EQUAL_CHAR('D', *rp);

}

int main(void)
{
    UNITY_BEGIN();

    RUN_TEST(test_init);
    RUN_TEST(test_implemented);
    RUN_TEST(test_full_empty);
    RUN_TEST(test_claim_all);
    RUN_TEST(test_reproduce_bug);
    RUN_TEST(test_write_without_reads);
    RUN_TEST(test_write_with_dual_overwrites);


    // RUN_TEST(test_claim_write);
    // RUN_TEST(test_claim_write__writes);
    // RUN_TEST(test_cancel_write__null);
    // RUN_TEST(test_cancel_write__non_null);
    // RUN_TEST(test_complete_write__null);
    // RUN_TEST(test_complete_write__non_null);

    // RUN_TEST(test_claim_read);
    // RUN_TEST(test_cancel_read__null);
    // RUN_TEST(test_cancel_read__non_null);
    // RUN_TEST(test_complete_read__null);
    // RUN_TEST(test_complete_read__non_null);

    UNITY_END();

    return 0;
}


// void test_claim_write(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     char *c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('A', *c);

//     c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('B', *c);

//     c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('C', *c);
//     TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&rb));

//     c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NULL(c);
//     TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&rb));
// }

// void test_claim_write__writes(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     char *c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('A', *c);
//     *c = 'P';

//     c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('B', *c);

//     char *c2 = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('C', *c2);

//     *c = 'Q';
//     *c2 = 'R';

//     TEST_ASSERT_NULL(retry_ringbuffer_claim_write_ptr(&la_rb));
//     TEST_ASSERT_EQUAL_STRING("PQR", buffer);
// }


// void test_cancel_write__null(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     char *c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('A', *c);

//     // passing NULL should cancel the last write
//     retry_ringbuffer_cancel_write(&la_rb, NULL);

//     c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('A', *c);

//     retry_ringbuffer_cancel_write(&la_rb, NULL);

//     // expect assertion failure: cannot cancel more than claimed
//     g_remaining_asserts = 1;
//     retry_ringbuffer_cancel_write(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(0, g_remaining_asserts);
// }

// void test_cancel_write__non_null(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     char *c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('A', *c);

//     // passing last claimed pointer should cancel this last write
//     retry_ringbuffer_cancel_write(&la_rb, c);

//     c = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('A', *c);

//     char *c2 = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('B', *c2);

//     // expect assertion failure: cannot cancel non-last write pointer
//     g_remaining_asserts = 1;
//     retry_ringbuffer_cancel_write(&la_rb, c);
//     TEST_ASSERT_EQUAL(0, g_remaining_asserts);
// }


// void test_complete_write__null(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     char *c1 = retry_ringbuffer_claim_write_ptr(&la_rb);
//     char *c2 = retry_ringbuffer_claim_write_ptr(&la_rb);
//     char *c3 = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c3);

//     *c1 = '1';
//     *c2 = '2';
//     *c3 = '3';
//     TEST_ASSERT(ringbuffer_is_empty(&rb));
//     TEST_ASSERT_EQUAL(0, ringbuffer_used_count(&rb));
//     retry_ringbuffer_complete_write(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(1, ringbuffer_used_count(&rb));
//     retry_ringbuffer_complete_write(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(2, ringbuffer_used_count(&rb));
//     retry_ringbuffer_complete_write(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(3, ringbuffer_used_count(&rb));
//     TEST_ASSERT(ringbuffer_is_full(&rb));

//     // overflow should only trigger if claim_write_ptr was called
//     // when the ringbuffer was full
//     TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&rb));
//     TEST_ASSERT_NULL(retry_ringbuffer_claim_write_ptr(&la_rb));
//     TEST_ASSERT_TRUE(ringbuffer_is_overflowed(&rb));

//     // overflow shoud reset after a succesful claim
//     ringbuffer_flush(&rb, 1);
//     TEST_ASSERT_NOT_NULL(retry_ringbuffer_claim_write_ptr(&la_rb));
//     retry_ringbuffer_complete_write(&la_rb, NULL);
//     TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&rb));
//     TEST_ASSERT(ringbuffer_is_full(&rb));

//     // expect assertion failure: cannot complete more than claimed
//     g_remaining_asserts = 1;
//     retry_ringbuffer_complete_write(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(0, g_remaining_asserts);
// }


// void test_complete_write__non_null(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     char *c1 = retry_ringbuffer_claim_write_ptr(&la_rb);
//     char *c2 = retry_ringbuffer_claim_write_ptr(&la_rb);
//     char *c3 = retry_ringbuffer_claim_write_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c3);

//     *c1 = '1';
//     *c2 = '2';
//     *c3 = '3';
//     TEST_ASSERT(ringbuffer_is_empty(&rb));
//     TEST_ASSERT_EQUAL(0, ringbuffer_used_count(&rb));
//     retry_ringbuffer_complete_write(&la_rb, c1);
//     TEST_ASSERT_EQUAL(1, ringbuffer_used_count(&rb));

//     // expect assertion failure: cannot complete out-of-order
//     g_remaining_asserts = 1;
//     retry_ringbuffer_complete_write(&la_rb, c3);
//     TEST_ASSERT_EQUAL(0, g_remaining_asserts);
// }


// void test_claim_read(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     // ringbuffer is empty: nothing to read
//     TEST_ASSERT_NULL(retry_ringbuffer_claim_read_ptr(&la_rb));

//     // write two elements to the ringbuffer
//     ringbuffer_write(&rb, "KL", 2);

//     char *c = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('K', *c);

//     c = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('L', *c);

//     // no more data to read
//     TEST_ASSERT_NULL(retry_ringbuffer_claim_read_ptr(&la_rb));

//     // write last element
//     ringbuffer_write(&rb, "M", 1);
//     TEST_ASSERT_TRUE(ringbuffer_is_full(&rb));
//     TEST_ASSERT_FALSE(ringbuffer_is_overflowed(&rb));

//     // overflow
//     ringbuffer_write(&rb, "N", 1);
//     TEST_ASSERT_TRUE(ringbuffer_is_full(&rb));
//     TEST_ASSERT_TRUE(ringbuffer_is_overflowed(&rb));

//     c = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('M', *c);


//     // no more reads available: prevous reads are still not completed
//     TEST_ASSERT_NULL(retry_ringbuffer_claim_read_ptr(&la_rb));

//     TEST_ASSERT_TRUE(ringbuffer_is_overflowed(&rb));
// }


// void test_cancel_read__null(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     ringbuffer_write(&rb, "KLM", 3);

//     char *c = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('K', *c);

//     // passing NULL should cancel the last read
//     retry_ringbuffer_cancel_read(&la_rb, NULL);

//     c = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('K', *c);

//     retry_ringbuffer_cancel_read(&la_rb, NULL);

//     // expect assertion failure: cannot cancel more than claimed
//     g_remaining_asserts = 1;
//     retry_ringbuffer_cancel_read(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(0, g_remaining_asserts);
// }

// void test_cancel_read__non_null(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     ringbuffer_write(&rb, "KLM", 3);

//     char *c = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('K', *c);

//     // passing last claimed pointer should cancel this last read
//     retry_ringbuffer_cancel_read(&la_rb, c);

//     c = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('K', *c);

//     char *c2 = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c);
//     TEST_ASSERT_EQUAL('L', *c2);

//     // expect assertion failure: cannot cancel non-last read pointer
//     g_remaining_asserts = 1;
//     retry_ringbuffer_cancel_read(&la_rb, c);
//     TEST_ASSERT_EQUAL(0, g_remaining_asserts);
// }


// void test_complete_read__null(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     ringbuffer_write(&rb, "123", 3);

//     char *c1 = retry_ringbuffer_claim_read_ptr(&la_rb);
//     char *c2 = retry_ringbuffer_claim_read_ptr(&la_rb);
//     char *c3 = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c1);
//     TEST_ASSERT_NOT_NULL(c2);
//     TEST_ASSERT_NOT_NULL(c3);
//     TEST_ASSERT_EQUAL(*c1, '1');
//     TEST_ASSERT_EQUAL(*c2, '2');
//     TEST_ASSERT_EQUAL(*c3, '3');

//     TEST_ASSERT(ringbuffer_is_full(&rb));
//     TEST_ASSERT_EQUAL(0, ringbuffer_free_count(&rb));
//     retry_ringbuffer_complete_read(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(1, ringbuffer_free_count(&rb));
//     retry_ringbuffer_complete_read(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(2, ringbuffer_free_count(&rb));
//     retry_ringbuffer_complete_read(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(3, ringbuffer_free_count(&rb));
//     TEST_ASSERT(ringbuffer_is_empty(&rb));

//     // expect assertion failure: cannot complete more than claimed
//     g_remaining_asserts = 1;
//     retry_ringbuffer_complete_read(&la_rb, NULL);
//     TEST_ASSERT_EQUAL(0, g_remaining_asserts);
// }


// void test_complete_read__non_null(void)
// {
//     g_remaining_asserts = 0;

//     char buffer[] = "ABC";
//     Ringbuffer rb;
//     ringbuffer_init(&rb, buffer, 1, 3);
//     RetryRingbuffer la_rb;
//     retry_ringbuffer_init(&la_rb, &rb);

//     ringbuffer_write(&rb, "123", 3);

//     char *c1 = retry_ringbuffer_claim_read_ptr(&la_rb);
//     char *c2 = retry_ringbuffer_claim_read_ptr(&la_rb);
//     char *c3 = retry_ringbuffer_claim_read_ptr(&la_rb);
//     TEST_ASSERT_NOT_NULL(c3);

//     *c1 = '1';
//     *c2 = '2';
//     *c3 = '3';
//     TEST_ASSERT(ringbuffer_is_full(&rb));
//     TEST_ASSERT_EQUAL(0, ringbuffer_free_count(&rb));
//     retry_ringbuffer_complete_read(&la_rb, c1);
//     TEST_ASSERT_EQUAL(1, ringbuffer_free_count(&rb));

//     // expect assertion failure: cannot complete out-of-order
//     g_remaining_asserts = 1;
//     retry_ringbuffer_complete_read(&la_rb, c3);
//     TEST_ASSERT_EQUAL(0, g_remaining_asserts);
// }

