#include "assert.h"

__attribute__ ((weak)) void assert(bool ok)
{
    while(!ok);
}
