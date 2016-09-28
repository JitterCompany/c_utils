#include "align.h"
#include <stddef.h>
#include <stdint.h>

void* align_4(void *unaligned)
{
    return align(unaligned, 4);
}

void* align(void *unaligned, size_t alignment)
{
    const uintptr_t ptr = (uintptr_t)unaligned;
    const size_t align_mask = alignment-1;

    const uintptr_t result = ((ptr + align_mask) & ~align_mask);

    return (void *)result;
}

