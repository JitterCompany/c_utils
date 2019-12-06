#ifndef C_UTILS_ARRAY_H
#define C_UTILS_ARRAY_H

#include <c_utils/static_assert.h>
#include <c_utils/max.h>



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


#define array_max(arr, length) \
    ({ __typeof__(&arr[0]) _curr_ptr = (arr); \
         __typeof__(arr[0]) _curr_max = arr[0]; \
         size_t _length = length; \
         for(size_t _i=0;_i<_length;_i++) { \
            _curr_max = max(_curr_max, *_curr_ptr++); \
        } \
        _curr_max; \
    })

#define array_min(arr, length) \
    ({ __typeof__(&arr[0]) _curr_ptr = (arr); \
         __typeof__(arr[0]) _curr_min = arr[0]; \
         size_t _length = length; \
         for(size_t _i=0;_i<_length;_i++) { \
            _curr_min = min(_curr_min, *_curr_ptr++); \
        } \
        _curr_min; \
    })

