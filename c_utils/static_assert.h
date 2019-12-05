#ifndef ASSERT_H
#define ASSERT_H

/* assert.h: provides a compile-time assertion macro
 *
 * usage: STATIC_ASSERT(statement)
 *
 * If a statement is false, the macro wil throw a compiler error of some sort.
 * A failed assertion will issue a division by zero error/warning
 */


#define ASSERT_CONCAT_(a, b) a##b
#define ASSERT_CONCAT(a, b) ASSERT_CONCAT_(a, b)
/* These can't be used after statements in c89. */
#ifdef __COUNTER__
#define STATIC_ASSERT(e) \
	    enum { ASSERT_CONCAT(c_utils_static_assert_, ASSERT_CONCAT(__COUNTER__, __LINE__)) = 1/(!!(e)) }
#else
/* This can't be used twice on the same line so ensure if using in headers
 *    * that the headers are not included twice (by wrapping in #ifndef...#endif)
 *       * Note it doesn't cause an issue when used on same line of separate modules
 *          * compiled with gcc -combine -fwhole-program.  */
#define STATIC_ASSERT(e) \
	    enum { ASSERT_CONCAT(c_utils_static_assert_, __LINE__) = 1/(!!(e)) }
#endif

#endif
