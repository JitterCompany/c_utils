#include "long_long_to_str.h"

#include <string.h>
#if defined(__linux__)
#include <bsd/string.h>
#endif

bool long_long_to_str(char *result_str, size_t result_size, long long val)
{
    // Edge case
    if(val == 0) {
        return (strlcpy(result_str, "0", result_size) < result_size);
    }

    char buff[64];
    memset(buff, 0, sizeof(buff));

    int i = 62;
    int sign = (val < 0);
    unsigned long long unsigned_val = val;
    if(sign) {
        unsigned_val = -val;
    }


    for(; unsigned_val && i ; --i, unsigned_val /= 10) {
        buff[i] = "0123456789"[unsigned_val % 10];
    }

    if(sign) {
        buff[i--] = '-';
    }

    return (strlcpy(result_str, &buff[i+1], result_size) < result_size);
}

