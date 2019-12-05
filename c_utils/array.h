#ifndef COUNTOF_H
#define COUNTOF_H

#include <c_utils/static_assert.h>



#define IS_ARRAY(arr) \
    (!__builtin_types_compatible_p(typeof((arr)), typeof(&(*arr))))

// Calculates length of array. Compiling will fail if you pass a non-array
// value.
#define array_length(arr) \
    ((sizeof(STATIC_ASSERT(IS_ARRAY(arr)))*0) + \
     (sizeof(arr) / sizeof(*arr)))

#endif

