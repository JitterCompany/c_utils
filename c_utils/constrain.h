#ifndef CONSTRAIN_H
#define CONSTRAIN_H

#include <c_utils/max.h>

#define constrain(x, lower_limit, upper_limit) \
    ({ __typeof__ (x) _x = (x); \
     __typeof__ (lower_limit) _lower_limit = (lower_limit); \
     __typeof__ (upper_limit) _upper_limit = (upper_limit); \
     min(max(_x, _lower_limit), _upper_limit); })

#endif

