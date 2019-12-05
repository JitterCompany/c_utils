#ifndef C_UTILS_ARRAY_H
#define C_UTILS_ARRAY_H

#include <c_utils/static_assert.h>



#define IS_ARRAY(arr) \
    (!__builtin_types_compatible_p(typeof((arr)), typeof(&(*arr))))

// Calculates length of array. Compiling will fail if you pass a non-array
// value.
#define array_length(arr) \
    ({ \
     STATIC_ASSERT(IS_ARRAY(arr)); \
     (sizeof(arr) / sizeof(*arr)); \
    })

#endif

