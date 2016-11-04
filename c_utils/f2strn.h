#ifndef F2STR_H
#define F2STR_H

/* Print float to string. Make sure to supply a buffer (str)
 * that is large enough for the result. If the length of the result is
 * equal to n-1, the buffer was too small and digits may have been lost.
 *
 * @param str       buffer to store the result. At least 16 bytes,
 *                  or more depending on the float that is parsed.
 * @param n         size of the buffer (str)
 *
 * @param num_dec   how many decimal places should be printed.
 *                  Same effect as the printf("%.<num_dec>")
 *
 * @return          The result string. If the lengh is n-1, the buffer
 *                  was too small to store the result.
 *                  To avoid buffer overflow one or more digits may have
 *                  been omitted.
 */
char *f2strn(float a, char *str, size_t n, const int num_dec);

#endif
