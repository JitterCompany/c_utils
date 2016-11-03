#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "static_assert.h"

/* These values are used by ringbuffer_is_initialized() to tell if a ringbuffer
 * has been initialized.
 * NOT_INITIALIZED is chosen to be zeroes because that is the most likely
 * case for an uninitialized ringbuffer stored in a global variable.
 *
 * In case ringbuffer is used for IPC between CPU cores for example, both
 * cores may use the same ringbuffer object in shared memory.
 * One of the cores initializes the ringbuffer, while the other can check
 * if this initialization was done.
 */
enum initialize_status {
    INITIALIZED = 0xC0DE,
    NOT_INITIALIZED = 0x0000
};

/*
 * Struct representing a ringbuffer 'object'.
 *
 * This contains all state associated with a ringbuffer.
 */
typedef struct {
    uint8_t *first_elem;                // address of the first element
    uint8_t *last_elem;                 // address of the last element
    uint8_t *volatile read;             // current read pointer
    uint8_t *volatile write;            // current write pointer
    uint32_t elem_sz;                   // element size: read/write pointers
                                            // advance in steps of this size
    volatile uint8_t readWrap;          // toggles when the read ptr wraps
    volatile uint8_t writeWrap;         // toggles when the write ptr wraps
    volatile bool overflow;             // last write attempt failed
    volatile uint16_t initialize_status;// is the ringbuffer is initialized?
} Ringbuffer;


/**
 * Initialize a ringbuffer object.
 *
 * The ringbuffer, once initialized with this function,
 * can be used with all the other ringbuffer_* functions.
 *
 * @param ringbuffer    Ringbuffer object that is to be initialized.
 *                      Note: if sharing a ringbuffer between multiple threads,
 *                      only initialize it once. Do not re-init while in use.
 *
 * @param data          A buffer where the ringbuffer data will be stored.
 *                      Allocate memory that stays valid for at least as long
 *                      as the ringbuffer object is used.
 *                      NOTE: the caller is responsible for properly aligning
 *                      this memory.
 *
 * @param element_size  Size in bytes of the elements to be stored in the
 *                      ringbuffer. Data may only be written to / read from the
 *                      ringbuffer in chuncks of this size.
 *
 * @param element_count Maximum amount of elements (of size element_size)
 *                      that can be stored in the ringbuffer.
 *                      NOTE: make sure the data parameter points to memory of
 *                      at least (element_size * element_count) bytes
 */
void ringbuffer_init(Ringbuffer *ringbuffer, void *data, size_t element_size
             , size_t element_count);


/**
 * Check if the given ringbuffer object is already initialized.
 *
 * @param ringbuffer    Ringbuffer object that may or may not be initialized
 *                      (@see ringbuffer_init)
 *
 * @return              True if the ringbuffer is initialized
 */
bool ringbuffer_is_initialized(Ringbuffer *ringbuffer);


/**
 * Find out the element size of the given ringbuffer.
 *
 * Useful if another thread / process has initialized the ringbuffer,
 * but the current context does not know its element size.
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              Element size. Reads or writes are always in chunks of
 *                      this size (in bytes).
 */
uint32_t ringbuffer_get_element_size(Ringbuffer *ringbuffer); // getter for element_size


/**
 * Clear all data in the ringbuffer.
 *
 * Note: this is only safe if you know no other context is using the ringbuffer.
 * If another thread is halfway reading some data, corruption will occur.
 * For a safe way to discard all data, @see ringbuffer_flush.
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 */
void ringbuffer_clear(Ringbuffer *ringbuffer);


/**
 * Simple copy-based function to write data to the ringbuffer.
 *
 * Copy up to element_count elements to the ringbuffer.
 * If not enough free space is available, not all elements may be copied.
 *
 * For writing to the ringbuffer without copy overhead,
 * @see ringbuffer_get_writeable()
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @param elements      array of data elements to be written to the buffer.
 *                      Should contain element_count * element_size bytes of
 *                      data (@see ringbuffer_get_element_size)
 *
 * @param element_count Amount of elements to copy
 *
 * @return              Amount of elements copied.
 */
uint32_t ringbuffer_write(Ringbuffer *ringbuffer,
                  const void *elements, uint32_t element_count);


/**
 * Simple copy-based function to read data from the ringbuffer.
 *
 * Pop up to element_count elements from the ringbuffer.
 * If not enough data is available, not all elements may be copied.
 *
 * For reading from the ringbuffer without copy overhead,
 * @see ringbuffer_get_readable()
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @param elements      array of data elements to be read from the buffer.
 *                      Should be at least of size element_count * element_size
 *                      (@see ringbuffer_get_element_size)
 *
 * @param element_count Amount of elements to copy
 *
 * @return              Amount of elements copied.
 */
uint32_t ringbuffer_read(Ringbuffer *ringbuffer, void *elements, uint32_t element_count);

/**
 * Remove up to element_count elements from the ringbuffer.
 *
 * This is equivalent to calling ringbuffer_advance up to n=element_count times.
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @param element_count Maximum amount of elements to discard.
 */
void ringbuffer_flush(Ringbuffer *ringbuffer, uint32_t element_count);


/**
 * Directly access the write pointer.
 *
 * This allows efficiently writing data right into the ringbuffer.
 * If you are done writing data, commit the changes with ringbuffer_commit().
 * Note: make sure to never write more than element_size bytes to the queue
 * (@see ringbuffer_get_element_size)
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              Pointer to a writeable chunk of memory of size
 *                      element_size. NULL if no space left.
 */
void *ringbuffer_get_writeable(Ringbuffer *ringbuffer);


/**
 * Commit: save data written via the write pointer
 * (@see ringbuffer_get_writeable).
 *
 * Commit advances the write pointer to the next chunk of data, effectively
 * handing the written data over to the queue. Never re-use old write pointers
 * after committing: as soon as ringbuffer_commit() is done,
 * another context may be reading that data!
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              True if the write pointer is succesfully advanced.
 *                      False if no more space is available: in this case,
 *                      the caller should wait untill more data is read and
 *                      try again.
 */
bool ringbuffer_commit(Ringbuffer *ringbuffer);


/**
 * Directly access the read pointer.
 *
 * This allows efficient access to data in the ringbuffer: no data is copied.
 * If you are done reading data, call ringbuffer_advance() to allow
 * the space to be re-used for the next write.
 * Note: make sure to never read more than element_size bytes from the queue
 * (@see ringbuffer_get_element_size)
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              Pointer to a readable chunk of memory of size
 *                      element_size. NULL if no data left.
 */
void *ringbuffer_get_readable(Ringbuffer *ringbuffer);


/**
 * Directly access the read pointer at an offset.
 *
 * This is similar to ringbuffer_get_readable(), but peeks the read pointer
 * at an offset of n chunks. The returned read pointer is the same as if
 * you would have called ringbuffer_advance() n=offset times.
 * Usefull if you want to peek ahead without advancing the read pointer.
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @param offset        Amount of chunks to skip ahead.
 *
 * @return              Pointer to a readable chunk of memory of size
 *                      element_size. NULL if no data left at this offset.
 */
void *ringbuffer_get_readable_offset(Ringbuffer *ringbuffer, uint32_t offset);


/**
 * Advance: notify the ringbuffer that you are done reading with the current
 * read pointer.
 *
 * The read pointer (@see ringbuffer_get_readable) is no longer valid
 * and the data may be overwritten the next time data is written.
 * Never re-use old read pointers after advancing: as soon as 
 * ringbuffer_advance() is done, another context may be overwriting that data!
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              True if the read pointer is succesfully advanced.
 *                      False if no more data is available: in this case,
 *                      the caller should wait untill more data is written and
 *                      try again.
 */
bool ringbuffer_advance(Ringbuffer *ringbuffer);


/**
 * Check if the ringbuffer is empty.
 *
 * If the ringbuffer contains no data it is empty. In this case, 
 * ringbuffer_advance() would fail and ringbuffer_get_readable()
 * would return a NULL pointer.
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              True if the ringbuffer is empty
 */
bool ringbuffer_is_empty(Ringbuffer *ringbuffer);


/**
 * Check if the ringbuffer is full.
 *
 * If the ringbuffer has no more space available for writing data, it is full.
 * In this case, ringbuffer_commit() would fail and ringbuffer_get_writeable() 
 * would return a NULL pointer.
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              True if the ringbuffer is full
 */
bool ringbuffer_is_full(Ringbuffer *ringbuffer);


/**
 * Check if an overflow would have occurred.
 *
 * If an attempt to write data is started while the ringbuffer is full, an
 * overflow flag is set. The ringbuffer stays marked as overflowed untill
 * it is no longer full.
 *
 * The code reading data from the ringbuffer can use this function to
 * discover if it is reading fast enough.
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              True if the ringbuffer is overflowed
 */
bool ringbuffer_is_overflowed(Ringbuffer *ringbuffer);


/**
 * Count the amount of elements that are available for writing
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              Amount of elements (of element_size bytes,
 *                      @see ringbuffer_get_element_size) available for writing.
 */
uint32_t ringbuffer_free_count(Ringbuffer *ringbuffer);


/**
 * Count the amount of elements in use: they are available to read
 *
 * @param ringbuffer    Initialized ringbuffer object (@see ringbuffer_init)
 *
 * @return              Amount of elements (of element_size bytes,
 *                      @see ringbuffer_get_element_size) available to read.
 */
uint32_t ringbuffer_used_count(Ringbuffer *ringbuffer);

// make sure the struct size is consistent on all compiles
// (example: it should be the same for both cores if used as IPC mechanism).
#ifndef TEST
#define RINGBUFFER_SIZE (28)
STATIC_ASSERT(sizeof(Ringbuffer) == RINGBUFFER_SIZE);
#endif

#endif

