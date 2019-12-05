#ifndef ROUND_H
#define ROUND_H

#define divide_round_up(a,b) \
    ({ __typeof__ (a) _a = (a); \
     __typeof__ (b) _b = (b); \
     ((a==0) ? 1 : ((_a - 1) / _b + 1)); })

#endif
