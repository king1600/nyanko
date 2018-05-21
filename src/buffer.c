#include "alloc.h"
#include "buffer.h"
#include <string.h>

void nk_buffer_free(nk_buffer_t* buf) {
    if (NK_LIKELY(buf->data))
        NK_FREE((void*) buf->data);
    buf->capacity = buf->read_pos = buf->write_pos = 0;
}

void nk_buffer_init(nk_buffer_t* buf, size_t init_size) {
    buf->capacity = init_size;
    buf->read_pos = buf->write_pos = 0;
    buf->data = (uint8_t*) NK_MALLOC(buf->capacity);
}

void nk_buffer_write_u64(nk_buffer_t* buf, const uint64_t data) {
    nk_buffer_write(buf, (const char*) &data, sizeof(uint64_t));
}

void nk_buffer_write(nk_buffer_t* buf, const char* data, size_t size) {
    if (NK_UNLIKELY(size == 0))
        return;

    if (NK_UNLIKELY(buf->read_pos != 0)) {
        memmove((void*) buf->data, (void*) &buf->data[buf->read_pos], nk_buffer_size(buf));
        buf->write_pos -= buf->read_pos;
        buf->read_pos = 0;
    }

    size_t resized = buf->write_pos + size;
    if (resized > buf->capacity) {
        buf->capacity = resized * 13 / 10;
        buf->data = (uint8_t*) NK_REALLOC(buf->data, buf->capacity);
    }

    memcpy((void*) &buf->data[buf->write_pos], (const void*) data, size);
    buf->write_pos = resized;
}