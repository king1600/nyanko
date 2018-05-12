#include "actor.h"
#include "alloc.h"
#include <string.h>

nk_value nk_alloc_data(size_t bytes) {
    return nk_gc_alloc(nk_actor_this(NULL), NK_TYPE_DATA, bytes);
}

nk_value nk_alloc_string(uint32_t bytes) {
    const size_t size = bytes + sizeof(uint32_t);
    nk_value value = nk_gc_alloc(nk_actor_this(NULL), NK_TYPE_DATA, size);
    *NK_PTR(uint32_t*, value) = bytes;
    return value;
}

nk_value nk_alloc_array(uint32_t items) {
    const size_t size = (sizeof(nk_value) * items) + sizeof(uint32_t);
    nk_value value = nk_gc_alloc(nk_actor_this(NULL), NK_TYPE_DATA, size);
    uint32_t* size_ptr = NK_PTR(uint32_t*, value);
    nk_value* array_ptr = (nk_value*) (size_ptr + 1);

    *size_ptr = items;
    while (items--)
        *array_ptr++ = NK_NULL;
        
    return value;
}

bool nk_get_array(nk_value value, nk_array_t* array) {
    if (NK_UNLIKELY(!nk_is_array(value)))
        return false;

    uint32_t* size_ptr = NK_PTR(uint32_t*, value);
    array->size = *size_ptr;
    array->data = (nk_value*) (size_ptr + 1);

    return true;
}

bool nk_get_string(nk_value value, nk_string_t* string) {
    if (NK_UNLIKELY(!nk_is_string(value)))
        return false;

    uint32_t* size_ptr = NK_PTR(uint32_t*, value);
    string->size = *size_ptr;
    string->data = (uint8_t*) (size_ptr + 1);

    return true;
}