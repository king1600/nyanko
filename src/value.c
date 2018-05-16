#include "actor.h"
#include "alloc.h"
#include <string.h>

#define NK_DECLARE_GET_SEQUENCE(size_type, data_type, list_type, name) \
    bool nk_get_##name(nk_value value, list_type list) {               \
        if (NK_UNLIKELY(!nk_is_##name(value) || list == NULL))         \
            return false;                                              \
                                                                       \
        size_type* size_ptr = NK_PTR(size_type*, value);               \
        list->size = *size_ptr;                                        \
        list->data = (data_type) &size_ptr[1];                         \
                                                                       \
        return true;                                                   \
    }

#define NK_DECLARE_ALLOC_SEQUENCE(size_type, data_type, name)               \
    nk_value nk_alloc_##name(size_type items) {                             \
        const size_t size = (sizeof(nk_value) * items) + sizeof(size_type); \
        nk_value value = nk_gc_alloc(nk_actor_this(NULL), data_type, size); \
                                                                            \
        size_type* size_ptr = NK_PTR(size_type*, value);                    \
        nk_value* data_ptr = (nk_value*) (size_ptr + 1);                    \
                                                                            \
        *size_ptr = items;                                                  \
        while (items--)                                                     \
            *data_ptr++ = NK_NULL;                                          \
                                                                            \
        return value;                                                       \
    }

NK_DECLARE_ALLOC_SEQUENCE(uint8_t, NK_TYPE_TUPLE, tuple)

NK_DECLARE_ALLOC_SEQUENCE(uint32_t, NK_TYPE_ARRAY, array)

NK_DECLARE_GET_SEQUENCE(uint8_t, nk_value*, nk_list_t*, tuple)

NK_DECLARE_GET_SEQUENCE(uint32_t, nk_value*, nk_list_t*, array)

NK_DECLARE_GET_SEQUENCE(uint32_t, uint8_t*, nk_string_t*, string)

nk_value nk_alloc_data(size_t bytes) {
    return nk_gc_alloc(nk_actor_this(NULL), NK_TYPE_DATA, bytes);
}

nk_value nk_alloc_string(uint32_t bytes) {
    const size_t size = bytes + sizeof(uint32_t);
    nk_value value = nk_gc_alloc(nk_actor_this(NULL), NK_TYPE_DATA, size);
    *NK_PTR(uint32_t*, value) = bytes;
    return value;
}