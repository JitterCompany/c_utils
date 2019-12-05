#ifndef MIN_H
#define MIN_H

#define min(a,b) \
    ({ __typeof__ (a) _min_a = (a); \
     __typeof__ (b) _min_b = (b); \
     _min_a < _min_b ? _min_a : _min_b; })

#endif

