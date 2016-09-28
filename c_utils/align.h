#ifndef ALIGN_H
#define ALIGN_H

#include <stddef.h>

/*
 * Align a pointer to a specified alignment. Alignment should be a power of 2.
 * Example: align(0x00000005, 4) == 0x00000008
 *
 * @param   unaligned       unaligned pointer
 * @param   alignment       power of two to which the pointer should be aligned.
 *
 * @return                  aligned pointer
 */
void *align(void *unaligned, size_t alignment);

/*
 * Align a pointer to a multiple of 4 bytes. Equivalent to align(unaligned, 4).
 */
void* align_4(void *unaligned);

#endif

