#include "retry_ringbuffer.h"
#include <assert.h>

#include <stdio.h>

static void debug(RetryRingbuffer *ctx)
{
    const RingbufferIndex read = ctx->ring->read;
    const RingbufferIndex write = ctx->ring->write;
    const RingbufferIndex next_write = ctx->next_write;
    const RingbufferIndex next_read = ctx->next_read;

    printf("---dbg\n");

    printf("read: %lu, wrap: %d\n", read.offset, read.wrap);
    printf("next_read: %lu, wrap: %d\n", next_read.offset, next_read.wrap);

    printf("write: %lu, wrap: %d\n", write.offset, write.wrap);
    printf("next_write: %lu, wrap: %d\n", next_write.offset, next_write.wrap);
    printf("---\n");
}

void retry_ringbuffer_init(RetryRingbuffer *ctx, Ringbuffer *ringbuffer)
{
    assert(ringbuffer_is_initialized(ringbuffer));
    ctx->ring = ringbuffer;

    ctx->next_write = ringbuffer->write;
    ctx->next_read = ringbuffer->read;
    ctx->num_reads = 0;
}

// return the next index relative to the supplied one
static RingbufferIndex next_index(const Ringbuffer *ringbuffer,
        RingbufferIndex index)
{
    index.offset+= ringbuffer->elem_sz;

    // index past end, wrap: move to first item, toggle wrap bit
    if(index.offset >= ringbuffer->num_bytes) {
        index.offset = 0;
        index.wrap^=1;
    }
    return index;
}

// return the previous index relative to the supplied one
static RingbufferIndex prev_index(const Ringbuffer *ringbuffer,
        RingbufferIndex index)
{
    index.offset-= ringbuffer->elem_sz;

    // index underflow, undo wrap: move to last item, toggle wrap bit
    if(index.offset >= ringbuffer->num_bytes) {
        index.offset = ringbuffer->num_bytes - ringbuffer->elem_sz;
        index.wrap^=1;
    }
    return index;
}

// return true if no more writeable space is available for claiming
inline bool retry_ringbuffer_is_full(RetryRingbuffer *ctx)
{

    const RingbufferIndex read = ctx->ring->read;
    const RingbufferIndex write = ctx->next_write;

    return ((read.offset == write.offset)
            && (read.wrap != write.wrap));
}

/**
 * Claim the first available write pointer
 *
 * Every time you claim a pointer, you get a pointer to the next available
 * writeable element in the ringbuffer (if any).
 * A claimed pointer is available for writing untill you complete or cancel
 * the write. See complete_write() and cancel_write().
 *
 * @return              Pointer to the next available writeable element
 *                      if available, NULL if no more elements are available.
 */
static void *retry_ringbuffer_claim_write_ptr(RetryRingbuffer *ctx)
{
    Ringbuffer *ring = ctx->ring;

    const bool full = retry_ringbuffer_is_full(ctx);
    ring->overflow = full;

    if(full) {
        return NULL;
    }


    const RingbufferIndex next_w = ctx->next_write;
    ctx->next_write = next_index(ring, next_w);

    return ring->first_elem + next_w.offset;
}


/**
 * Cancel the latest read
 *
 * Cancel reading from the pointer previously claimed with claim_read_ptr().
 *
 * NOTE: cancels (and conpletes) should be done in order: only the latest
 * read (that is not yet canceled or completed) can be canceled.
 * If you want to cancel multiple reads: first cancel the last read,
 * then the one before it, etc.
 *
 * @param read_ptr      read pointer as returned from claim_read_ptr().
 *                      This is used to perform a sanity check: cancels should
 *                      be done in the right order.
 *                      NULL may be passed in case you don't have access to
 *                      the read_ptr. In this case the check is skipped.
 */
static void retry_ringbuffer_cancel_read(RetryRingbuffer *ctx, const void *read_ptr)
{
    const Ringbuffer *ring = ctx->ring;
    const RingbufferIndex prev = prev_index(ring, ctx->next_read);

    // assertion: cannot cancel more reads than claimed
    assert(ctx->next_read.raw != ring->read.raw);

    if(read_ptr) {
        // assertion: canceled read should be the last claimed read
        assert(read_ptr == (ring->first_elem + prev.offset));
    }

    ctx->next_read = prev;
    ctx->num_reads -= 1;

}

/**
 * Complete the latest read
 *
 * Complete reading from the read pointer previously claimed with
 * claim_read_ptr().
 *
 * NOTE: completes (and cancels) should be done in order: only the first
 * read (that is not yet canceled or completed) can be completed.
 * If you want to complete multiple reads: first complete the oldest read,
 * then the one after it, etc.
 *
 * @param read_ptr      read pointer as returned from claim_read_ptr().
 *                      This is used to perform a sanity check: completes
 *                      should be done in the right order.
 *                      NULL may be passed in case you don't have access to
 *                      the read_ptr. In this case the check is skipped.
 */
static void retry_ringbuffer_complete_read(RetryRingbuffer *ctx, const void *read_ptr)
{
    if(read_ptr) {
        // assertion: completed read should be the first non-completed read
        assert(ringbuffer_get_readable(ctx->ring) == read_ptr);
    }

    // advance: assert the ringbuffer is not empty.
    // the ringbuffer should never be empty at this point,
    // because the read_ptr was claimed earlier.
    assert(ringbuffer_advance(ctx->ring));

    ctx->num_reads -= 1;
}


// return true if no more readable space is available for claiming
bool retry_ringbuffer_is_empty(RetryRingbuffer *ctx)
{
    const RingbufferIndex read = ctx->next_read;
    const RingbufferIndex write = ctx->ring->write;

    return (read.raw == write.raw);
}

void retry_ringbuffer_cancel_write(RetryRingbuffer *ctx, void *write_ptr)
{
    const Ringbuffer *ring = ctx->ring;
    const RingbufferIndex prev = prev_index(ring, ctx->next_write);

    // assertion: cannot cancel more writes than claimed
    assert(ctx->next_write.raw != ring->write.raw);

    if(write_ptr) {
        // assertion: canceled write should be the last claimed write
        assert(write_ptr == (ring->first_elem + prev.offset));
    }

    ctx->next_write = prev;
}

void retry_ringbuffer_complete_write(RetryRingbuffer *ctx, void *write_ptr)
{
    if(write_ptr) {
        // assertion: completed write should be the first non-completed write
        assert(ringbuffer_get_writeable(ctx->ring) == write_ptr);
    }

    // commit: assert the ringbuffer is not full.
    // the ringbuffer should never be full at this point,
    // because the write_ptr was claimed earlier.
    assert(ringbuffer_commit(ctx->ring));
}

void *retry_ringbuffer_claim_read_ptr(RetryRingbuffer *ctx)
{
    Ringbuffer *ring = ctx->ring;

    if(retry_ringbuffer_is_empty(ctx)) {
        return NULL;
    }

    const RingbufferIndex next_r = ctx->next_read;
    ctx->next_read = next_index(ring, next_r);

    ctx->num_reads += 1;

    // debug(ctx);

    return ring->first_elem + next_r.offset;
}

void retry_ringbuffer_complete_all_reads(RetryRingbuffer *ctx)
{
    for(size_t i=0; i<ctx->num_reads; i++) {
        retry_ringbuffer_complete_read(ctx, NULL);
    }
}

void retry_ringbuffer_cancel_all_reads(RetryRingbuffer *ctx)
{
    for(size_t i=0; i<ctx->num_reads; i++) {
        retry_ringbuffer_cancel_read(ctx, NULL);
    }
}

void *retry_ringbuffer_wrapping_write_ptr(RetryRingbuffer *ctx)
{
    void *dst = retry_ringbuffer_claim_write_ptr(ctx);
    if (!dst) {

        // We didn't get a write pointer, this means
        // that the ringbuffer is full. We want to overwrite the oldest entry.
        // However maybe there is already a read pointer given out for this element.
        // If so, we will complete/advance it.
        // If not we will claim a new read pointer and immediately complete it.
        void *rp = NULL;
        if (ctx->num_reads == 0) {
            rp = retry_ringbuffer_claim_read_ptr(ctx);
        }
        retry_ringbuffer_complete_read(ctx, rp);
        dst = retry_ringbuffer_claim_write_ptr(ctx);
    }

    // debug(ctx);

    return dst;
}
