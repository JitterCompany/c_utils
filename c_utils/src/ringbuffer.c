#include "ringbuffer.h"
#include "assert.h"
#include "string.h"

void ringbuffer_init(Ringbuffer *ringbuffer,
        void *data, size_t element_size, size_t element_count)
{
    ringbuffer->first_elem = (uint8_t *)data;
    // TODO detect overflow on num_bytes: first_elem + num_bytes should be ok
    ringbuffer->num_bytes = element_count * element_size;
    ringbuffer->elem_sz = element_size;

    ringbuffer_clear(ringbuffer);
    ringbuffer->initialize_status = INITIALIZED;
}

uint32_t ringbuffer_get_element_size(Ringbuffer *ringbuffer)
{
    return ringbuffer->elem_sz;
}

void ringbuffer_clear(Ringbuffer *ringbuffer)
{
    ringbuffer->read.raw = 0;
    ringbuffer->write.raw = 0;
    ringbuffer->overflow = false;
}

// return the next index relative to the supplied one
static RingbufferIndex next_index(const Ringbuffer *ringbuffer,
        RingbufferIndex index)
{
    index.offset+= ringbuffer->elem_sz;
    if(index.offset >= ringbuffer->num_bytes) {
        index.offset = 0;
        index.wrap^=1;
    }
    return index;
}

bool ringbuffer_advance(Ringbuffer *ringbuffer)
{
    if(ringbuffer_is_empty(ringbuffer)) {
        return false;
    }

    // update read pointer to the next element
    ringbuffer->read = next_index(ringbuffer, ringbuffer->read);

    return true;
}

void *ringbuffer_get_writeable(Ringbuffer *ringbuffer)
{
    bool full = ringbuffer_is_full(ringbuffer);
    ringbuffer->overflow = full;

    if(full) {
        return NULL;
    }
    return ringbuffer->first_elem + ringbuffer->write.offset;
}

uint32_t ringbuffer_write(Ringbuffer *ringbuffer,
                  const void *elements, uint32_t element_count)
{
    void *write;
    const uint8_t *elems = elements;
    uint32_t written = 0;
    while((written < element_count)
            && (write = ringbuffer_get_writeable(ringbuffer))) {
        memcpy(write, elems, ringbuffer->elem_sz);
        elems+= ringbuffer->elem_sz;
        ringbuffer_commit(ringbuffer);
        written++;
    }
    return written;
}

uint32_t ringbuffer_read(Ringbuffer *ringbuffer,
        void *elements, uint32_t element_count)
{
    void *read;
    uint8_t *elems = elements;
    uint32_t elements_read = 0;
    while((elements_read < element_count)
            && (read = ringbuffer_get_readable(ringbuffer))) {
        memcpy(elems, read, ringbuffer->elem_sz);
        elems+= ringbuffer->elem_sz;
        ringbuffer_advance(ringbuffer);
        elements_read++;
    }
    return elements_read;
}

void ringbuffer_flush(Ringbuffer *ringbuffer, uint32_t element_count)
{
    while(element_count) {
        if(!ringbuffer_advance(ringbuffer)) {
            break;
        }
        element_count--;
    }
}

bool ringbuffer_commit(Ringbuffer *ringbuffer)
{
    if(ringbuffer_is_full(ringbuffer)) {
        return false;
    }
    
    // update write pointer to the next element
    ringbuffer->write = next_index(ringbuffer, ringbuffer->write);
    return true;
}

bool ringbuffer_is_initialized(Ringbuffer *ringbuffer)
{
    return ringbuffer->initialize_status == INITIALIZED;
}

void *ringbuffer_get_readable(Ringbuffer *ringbuffer)
{
    if(ringbuffer_is_empty(ringbuffer)) {
       return NULL;
    }
    return ringbuffer->first_elem + ringbuffer->read.offset;
}

void *ringbuffer_get_readable_offset(Ringbuffer *ringbuffer, uint32_t offset)
{
    if(offset >= ringbuffer_used_count(ringbuffer)) {
        return NULL;
    }

    size_t byte_offset = (offset * ringbuffer->elem_sz)
        + ringbuffer->read.offset;
    
    // wrap around
    if(byte_offset >= ringbuffer->num_bytes) {
        byte_offset-= ringbuffer->num_bytes;
    }
    
    assert(byte_offset < ringbuffer->num_bytes);

    return ringbuffer->first_elem + byte_offset;
}

inline bool ringbuffer_is_empty(Ringbuffer *ringbuffer)
{
    const RingbufferIndex read = ringbuffer->read;
    const RingbufferIndex write = ringbuffer->write;

    return (read.raw == write.raw);
}

inline bool ringbuffer_is_full(Ringbuffer *ringbuffer)
{
    const RingbufferIndex read = ringbuffer->read;
    const RingbufferIndex write = ringbuffer->write;
    
    return ((read.offset == write.offset)
            && (read.wrap != write.wrap));
}

inline bool ringbuffer_is_overflowed(Ringbuffer *ringbuffer)
{
    return ringbuffer->overflow && ringbuffer_is_full(ringbuffer);
}

uint32_t ringbuffer_free_count(Ringbuffer *ringbuffer)
{
    const size_t max_free = ringbuffer->num_bytes / ringbuffer->elem_sz;

    return (max_free - ringbuffer_used_count(ringbuffer));
}

uint32_t ringbuffer_used_count(Ringbuffer *ringbuffer)
{
    // empty is a special case: r/w offsets are equal, but wrap bits too!
    if(ringbuffer_is_empty(ringbuffer)) {
        return 0;
    }

    const RingbufferIndex read = ringbuffer->read;
    const RingbufferIndex write = ringbuffer->write;

    size_t diff = write.offset - read.offset;
    // difference zero or underflow: compensate for wraparound (or full)
    if(!diff || (diff >= ringbuffer->num_bytes)) {
        diff+= ringbuffer->num_bytes;
    }

    return (diff / ringbuffer->elem_sz);
}

