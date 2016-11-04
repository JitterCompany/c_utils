#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

//
// fcvt.c
//
// Floating point to string conversion routines
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors
//    may be used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//

#include <math.h>

//
// cvt.c - IEEE floating point formatting routines for FreeBSD
// from GNU libc-4.6.27
//

static char *cvt(double arg, int ndigits, int *decpt, int *sign,
                 char *buf, int buff_size, int eflag)
{
    int r2;
    double fi, fj;
    char *p, *p1;

    if (ndigits < 0) ndigits = 0;
    if (ndigits >= buff_size - 1) ndigits = buff_size - 2;
    r2 = 0;
    *sign = 0;
    p = &buf[0];
    if (arg < 0) {
        *sign = 1;
        arg = -arg;
    }
    arg = modf(arg, &fi);
    p1 = &buf[buff_size];

    if (fi != 0) {
        p1 = &buf[buff_size];
        while (fi != 0) {
            fj = modf(fi / 10, &fi);
            *--p1 = (int)((fj + .03) * 10) + '0';
            r2++;
        }
        while (p1 < &buf[buff_size]) *p++ = *p1++;
    } else if (arg > 0) {
        while ((fj = arg * 10) < 1) {
            arg = fj;
            r2--;
        }
    }
    p1 = &buf[ndigits];
    if (eflag == 0) p1 += r2;
    *decpt = r2;
    if (p1 < &buf[0]) {
        buf[0] = '\0';
        return buf;
    }
    while (p <= p1 && p < &buf[buff_size]) {
        arg *= 10;
        arg = modf(arg, &fj);
        *p++ = (int) fj + '0';
    }
    if (p1 >= &buf[buff_size]) {
        buf[buff_size - 1] = '\0';
        return buf;
    }
    p = p1;
    *p1 += 5;
    while (*p1 > '9') {
        *p1 = '0';
        if (p1 > buf) {
            ++*--p1;
        } else {
            *p1 = '1';
            (*decpt)++;
            if (eflag == 0) {
                if (p > buf) *p = '0';
                p++;
            }
        }
    }
    *p = '\0';
    return buf;
}

char *f2strn(float a, char *str, int n, const int num_dec)
{
    if(isinf(a)) {
        if(a < 0) {
            strcpy(str, "-inf");
        } else {
            strcpy(str, "inf");
        }
        return str;
    }

    if(isnan(a)) {
        strcpy(str, "nan");
        return str;
    }

    char convert_buffer[n];
    int dec, sign;
    char *converted = cvt(a, num_dec, &dec, &sign, convert_buffer, n, 0);

    bool zero_prefix = false;
    size_t zeroes_prefix_count = 0;
    size_t decimals_prefix_count = 0;
    size_t decimals_max_count = 0;

    size_t max_strlen = n-(1+1+2); // null terminator, sign, '0.' prefix

    if(dec <= 0) {
        zeroes_prefix_count = (-1*dec);

        // limit values to avoid buffer overflow
        if(zeroes_prefix_count > max_strlen) {
            zeroes_prefix_count = max_strlen;
        }
        decimals_max_count = max_strlen - zeroes_prefix_count;
        zero_prefix = true;
    } else {
        decimals_prefix_count = dec;

        // limit values to avoid buffer overflow
        if(decimals_prefix_count > max_strlen) {
            decimals_prefix_count = max_strlen;
        }
        decimals_max_count = max_strlen - decimals_prefix_count;
    }

    char *result = str;
    char *prefix = result;

    // negative number: add prefix '-'
    if(sign) {
        *(result++) = '-';
        prefix++;
    }

    // negative amount of digits before the '.':
    // add prefix 0.<n times '0'>
    if(zero_prefix) {
        strcpy(result, "0.");
        result+= 2;

        memset(result, '0', zeroes_prefix_count);
        result+= zeroes_prefix_count;
    }
    // copy the int part and add the '.' separator
    else {
        strncpy(result, converted, decimals_prefix_count);
        result+= decimals_prefix_count;
        *(result++) = '.';
    }

    // copy the decimals
    strncpy(result, (converted+decimals_prefix_count), decimals_max_count);
    result+= decimals_max_count;

    // force a null-terminator
    *(result++) = '\0';


    return str;
}

