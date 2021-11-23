#ifndef RETRY_RINGBUFFER_H
#define RETRY_RINGBUFFER_H

#include "ringbuffer.h"

/** retry_ringbuffer: use a standard ringbuffer (@see c_utils package) with mcu_la.
 *
 * RetryRingbuffer is an access layer to allow mcu_la to access a ringbuffer.
 * This layer is required because the la library wants to do some advanced
 * stuff such as claiming multipe write (or read) pointers ahead.
 *
 * Note: never mix writing to a ringbuffer via retry_ringbuffer with writing
 * directly to the ringbuffer.
 * Similarly, never mix reading via retry_ringbuffer with reading directly
 * from the ringbuffer.
 */


typedef struct retry_ringbuffer RetryRingbuffer;


/**
 * Initialize a RetryRingbuffer object.
 *
 * @param ctx           RetryRingbuffer object to initialize.
 *                      This object holds all the state and should be passed
 *                      to all other retry_ringbuffer_ functions.
 *
 * @param ringbuffer    Normal ringbuffer, @see package c_utils/ringbuffer.h.
 *                      NOTE: this ringbuffer should have been initialized
 *                      with ringbuffer_init()!
 */
void retry_ringbuffer_init(RetryRingbuffer *ctx, Ringbuffer *ringbuffer);


/**
 * Cancel the latest write
 *
 * Cancel writing to the pointer previously claimed with claim_write_ptr().
 *
 * NOTE: cancels (and conpletes) should be done in order: only the latest
 * write (that is not yet canceled or completed) can be canceled.
 * If you want to cancel multiple writes: first cancel the last write,
 * then the one before it, etc.
 *
 * @param write_ptr     Write pointer as returned from claim_write_ptr().
 *                      This is used to perform a sanity check: cancels should
 *                      be done in the right order.
 *                      NULL may be passed in case you don't have access to
 *                      the write_ptr. In this case the check is skipped.
 */
void retry_ringbuffer_cancel_write(RetryRingbuffer *ctx, void *write_ptr);


/**
 * Complete the latest write
 *
 * Complete writing to the write pointer previously claimed with
 * claim_write_ptr().
 *
 * NOTE: completes (and cancels) should be done in order: only the first
 * write (that is not yet canceled or completed) can be completed.
 * If you want to complete multiple writes: first complete the oldest write,
 * then the one after it, etc.
 *
 * @param write_ptr     Write pointer as returned from claim_write_ptr().
 *                      This is used to perform a sanity check: completes
 *                      should be done in the right order.
 *                      NULL may be passed in case you don't have access to
 *                      the write_ptr. In this case the check is skipped.
 */
void retry_ringbuffer_complete_write(RetryRingbuffer *ctx, void *write_ptr);

/**
 * Claim the first available read pointer
 *
 * Every time you claim a pointer, you get a pointer to the next available
 * readable element in the ringbuffer (if any).
 * A claimed pointer is available for reading untill you complete or cancel
 * the read. See complete_read() and cancel_read().
 *
 * @return              Pointer to the next available readable element
 *                      if available, NULL if no more elements are available.
 */
void *retry_ringbuffer_claim_read_ptr(RetryRingbuffer *ctx);


/**
 * Claim the first available write pointer, or replace oldest element
 *
 * Every time you claim a pointer, you get a pointer to the next available
 * writeable element in the ringbuffer (if any).
 * A claimed pointer is available for writing untill you complete or cancel
 * the write. See complete_write() and cancel_write().
 *
 * @return              Pointer to the next writeable element
 *
 */
void* retry_ringbuffer_wrapping_write_ptr(RetryRingbuffer *ctx);

/**
 * Mark all outstanding read pointers as completed.
 * This means the slots will be writable again without discarding elements.
 */
void retry_ringbuffer_complete_all_reads(RetryRingbuffer *ctx);


/**
 * Cancel all outstanding readpointers.
 * This means they will come available again for claiming
 */
void retry_ringbuffer_cancel_all_reads(RetryRingbuffer *ctx);

/**
 * Returns true if there are zero elements in the ringbuffer
 */
bool retry_ringbuffer_is_empty(RetryRingbuffer *ctx);

/**
 * Returns true if no more elements are available
 */
bool retry_ringbuffer_is_full(RetryRingbuffer *ctx);

struct retry_ringbuffer {
    Ringbuffer *ring;
    volatile RingbufferIndex next_write;
    volatile RingbufferIndex next_read;
    volatile size_t num_reads;
};


#endif
