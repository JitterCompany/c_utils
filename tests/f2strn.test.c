#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "f2strn.h"
#include "unity.h"
#include <stdlib.h>

// Set to 1 to run all possible floats trough the test.
// Note: depending on your hardware this test may take several hours!
#define RUN_BRUTEFORCE_TEST (0)

void TEST_ASSERT_FLOATS_EXACTLY_THE_SAME(float reference, float testvalue)
{
    bool match = (reference == testvalue)
                 || (isnan(reference) && isnan(testvalue));

    char msg[256];
    snprintf(msg, sizeof(msg),
             "(floats not the same: expected %.50f, got %.50f)",
             reference, testvalue);
    TEST_ASSERT_TRUE_MESSAGE(match, msg);
}

void test_f2strn__hardcoded_floats__match(void)
{
    char string[256];
    memset(string, 0, 256);

    char buf[256];
    TEST_ASSERT_EQUAL_STRING("3.14",  f2strn(3.14f, string, 256, 2));

    float a = 7.98;
    snprintf(buf, sizeof(buf), "%.12f", a);
    TEST_ASSERT_EQUAL_STRING(buf, f2strn(a, string, 256, 12));

    a = 7.9809;
    snprintf(buf, sizeof(buf), "%.12f", a);
    TEST_ASSERT_EQUAL_STRING(buf, f2strn(a, string, 256, 12));

    TEST_ASSERT_EQUAL_STRING("345.789", f2strn(345.789f, string, 256, 3));


    a = 345.789140f;
    snprintf(buf, 256, "%.6f", a);
    TEST_ASSERT_EQUAL_STRING(buf,f2strn(a, string, 256, 6));

    a = 3452345.0f;
    snprintf(buf, 256, "%.1f", a);
    TEST_ASSERT_EQUAL_STRING(buf,f2strn(a, string, 256, 1));

    a = 223452345.0f;
    snprintf(buf, 256, "%.1f", a);
    TEST_ASSERT_EQUAL_STRING(buf,f2strn(a, string, 256, 1));

    a = 1e1f;
    snprintf(buf, 256, "%.1f", a);
    TEST_ASSERT_EQUAL_STRING(buf,f2strn(a, string, 256, 1));

    a = 0.009f;
    snprintf(buf, 256, "%.12f", a);
    TEST_ASSERT_EQUAL_STRING(buf,f2strn(a, string, 256, 12));

    a = 0.00000000000000000000000000000000000002074406015f;
    snprintf(buf, 256, "%.47f", a);
    size_t buf_len = strlen(buf);
    char *result = f2strn(a, string, 256, 47);
    TEST_ASSERT_EQUAL(buf_len, strlen(result));
    if(buf_len >= 47) {
        buf[buf_len-1] = '\0';
        result[buf_len-1] = '\0';
    }
    TEST_ASSERT_EQUAL_STRING(buf, result);
}


void test_f2strn__infinity__returns_inf(void)
{
    char string[256];
    memset(string, 0, 256);

    float a = INFINITY;
    char buf[256];
    snprintf(buf, 256, "%.8f", a);

    TEST_ASSERT_EQUAL_STRING(buf,f2strn(a, string, 256, 1));
}

void test_f2strn__not_a_number__returns_nan(void)
{
    char string[256];
    memset(string, 0, 256);

    float a = NAN;
    char buf[256];
    snprintf(buf, 256, "%.8f", a);

    TEST_ASSERT_EQUAL_STRING(buf,f2strn(a, string, 256, 1));
}

void print_bruteforce_float(char *prefix, float n)
{
    printf("%s", prefix);
    printf(": %.50f\n", n);
}

void test_f2strn__bruteforce__match(void)
{
    union float_bytes {
        float val;
        unsigned char bytes[sizeof(float)];
    } data;

    data.val = 0.0f;

    for(int i=0; i<256; i++) {
        data.bytes[0] = i;

        for(int j=0; j<256; j++) {
            data.bytes[1] = j;
            print_bruteforce_float("PROGRESS", (float)((256*i)+j)/(65536.0f));

            for(int k=0; k<256; k++) {
                data.bytes[2] = k;

                for(int l=0; l<256; l++) {
                    data.bytes[3] = l;

                    char buffer[256];
                    char string[256];

                    snprintf(buffer, sizeof(buffer), "%.48f", data.val);

                    char *result = f2strn(data.val, string, 256, 48);

                    char *ref_endptr;
                    char *calculated_endptr;
                    float reference = strtof(buffer, &ref_endptr);
                    float calculated = strtof(result, &calculated_endptr);
                    TEST_ASSERT_TRUE(ref_endptr > buffer);
                    TEST_ASSERT_TRUE(calculated_endptr > result);

                    TEST_ASSERT_FLOATS_EXACTLY_THE_SAME(reference, calculated);

                    bool match = (reference == calculated)
                                 || (isnan(reference) && isnan(calculated));
                    if(!match) {
                        break;
                    }
                    //TEST_ASSERT_EQUAL_STRING(buffer, result);

                }
            }
        }
    }
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_f2strn__hardcoded_floats__match);
    RUN_TEST(test_f2strn__infinity__returns_inf);
    RUN_TEST(test_f2strn__not_a_number__returns_nan);
#if RUN_BRUTEFORCE_TEST
    RUN_TEST(test_f2strn__bruteforce__match);
#endif
    UNITY_END();



    return 0;
}
