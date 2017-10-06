#ifndef C_UTILS_ASSERT_H
#define C_UTILS_ASSERT_H

#include <stdbool.h>

/* assert.h: provides a weak (overrideable) assert() implementation
 *
 * usage: assert(statement)
 *
 * If a statement is false, the function will block forever.
 * A program may override the implementation to do better error handling.
 */
void assert(bool ok);

#endif
