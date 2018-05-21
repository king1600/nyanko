#ifndef _NK_BUFFER_H
#define _NK_BUFFER_H

#include "nk.h"

typedef struct {
    uint8_t* data;
    size_t capacity;
    size_t read_pos;
    size_t write_pos;
} nk_buffer_t;

void nk_buffer_free(nk_buffer_t* buf);

void nk_buffer_init(nk_buffer_t* buf, size_t init_size);

void nk_buffer_write_u64(nk_buffer_t* buf, const uint64_t data);

void nk_buffer_write(nk_buffer_t* buf, const char* data, size_t size);

static NK_INLINE size_t nk_buffer_size(nk_buffer_t* buffer) {
    return buffer->write_pos - buffer->read_pos;
}

#endif // _NK_BUFFER_H