#include "ringbuffer.h"
#include "string.h"

void ringbuffer_init(Ringbuffer *ringbuffer,
        void *data, size_t element_size, size_t element_count)
{
    ringbuffer->first_elem = (uint8_t *)data;
    ringbuffer->last_elem = (ringbuffer->first_elem +
            (element_size * (element_count-1)));
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
    ringbuffer->read = ringbuffer->first_elem;
    ringbuffer->write = ringbuffer->first_elem;
    ringbuffer->read_wrap = 0;
    ringbuffer->write_wrap = 0;
    ringbuffer->overflow = false;
}

bool ringbuffer_advance(Ringbuffer *ringbuffer)
{
    if(!ringbuffer_is_empty(ringbuffer)) {
        if(ringbuffer->read < ringbuffer->last_elem) {
            ringbuffer->read+= ringbuffer->elem_sz;
        } else {
            ringbuffer->read = ringbuffer->first_elem;
            ringbuffer->read_wrap^=1;
        }
        return true;
    }
    return false;
}

void *ringbuffer_get_writeable(Ringbuffer *ringbuffer)
{
    void *result = NULL;
    bool full = ringbuffer_is_full(ringbuffer);
    ringbuffer->overflow = full;

    if(!full) {
        result = ringbuffer->write;
    }
    return result;
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
    if(!ringbuffer_is_full(ringbuffer)) {
        if(ringbuffer->write < ringbuffer->last_elem) {
            ringbuffer->write+= ringbuffer->elem_sz;
        } else {
            ringbuffer->write = ringbuffer->first_elem;
            ringbuffer->write_wrap^=1;
        }
        return true;
    }
    return false;
}

bool ringbuffer_is_initialized(Ringbuffer *ringbuffer)
{
    return ringbuffer->initialize_status == INITIALIZED;
}

void *ringbuffer_get_readable(Ringbuffer *ringbuffer)
{
    return ringbuffer_is_empty(ringbuffer) ? NULL : ringbuffer->read;
}

void *ringbuffer_get_readable_offset(Ringbuffer *ringbuffer, uint32_t offset)
{
    uint8_t *ptr = ringbuffer->read + (offset * ringbuffer->elem_sz);

    // wrap around
    ptr = (ptr > ringbuffer->last_elem)
          ? (ringbuffer->first_elem
             + ((ptr - ringbuffer->last_elem) - ringbuffer->elem_sz))
          : ptr;

    // return NULL if offset is invalid
    // (out of range or not enough data to read)
    return (ptr > ringbuffer->last_elem || ptr < ringbuffer->first_elem)
           ? NULL
           : ((ringbuffer_used_count(ringbuffer) > offset) ? ptr : NULL);
}

inline bool ringbuffer_is_empty(Ringbuffer *ringbuffer)
{
    return (ringbuffer->read == ringbuffer->write
            && ringbuffer->read_wrap == ringbuffer->write_wrap);
}

inline bool ringbuffer_is_full(Ringbuffer *ringbuffer)
{
    return (ringbuffer->read == ringbuffer->write
            && ringbuffer->read_wrap != ringbuffer->write_wrap);
}

inline bool ringbuffer_is_overflowed(Ringbuffer *ringbuffer)
{
    return ringbuffer->overflow && ringbuffer_is_full(ringbuffer);
}

uint32_t ringbuffer_free_count(Ringbuffer *ringbuffer)
{
    uint32_t max_free = 1 + ((ringbuffer->last_elem - ringbuffer->first_elem)
                        / ringbuffer->elem_sz);

    return (max_free - ringbuffer_used_count(ringbuffer));
}

uint32_t ringbuffer_used_count(Ringbuffer *ringbuffer)
{
    // check for empty first to avoid race conflict:
    // queue may become non-empty at any time if another thread/context
    // performs a write.
    if(ringbuffer_is_empty(ringbuffer)) {
        return 0;
    }

    uint32_t maxDiff = ringbuffer->last_elem - ringbuffer->first_elem;
    uint32_t diff = ringbuffer->write - ringbuffer->read;
    if(diff && (diff <= maxDiff)) {
        return (diff / ringbuffer->elem_sz);
    }
    // write pointer <= read pointer: compensate for wraparound
    diff+= (maxDiff + ringbuffer->elem_sz);
    return (diff / ringbuffer->elem_sz);
}

