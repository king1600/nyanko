#include "gc.h"
#include "alloc.h"
#include "atomic.h"
#include <string.h>

#define NK_SLOT_SIZE sizeof(nk_slot_t)
#define NK_GC_BITS (sizeof(uint64_t) << 3)
#define NK_GC_INDEX(map, slot) (map)[(slot) / NK_GC_BITS]
#define NK_GC_SLOT_BIT(slot) (1ULL << ((slot) & (NK_GC_BITS - 1)))
#define NK_GC_IS_SET(map, slot) (NK_GC_INDEX(map, slot) & NK_GC_SLOT_BIT(slot))
#define NK_GC_TOGGLE(map, slot) (NK_GC_INDEX(map, slot) ^= NK_GC_SLOT_BIT(slot))

// traverse an nk_value
#define NK_GC_TRAVERSE(obj, handle) do {                    \
    nk_value value;                                         \
    nk_array_t array;                                       \
    nk_map_iter_t it;                                       \
    switch (NK_TYPE(obj)) {                                 \
        case NK_TYPE_ARRAY:                                 \
            if (NK_LIKELY(nk_get_array(obj, &array)))       \
                for (uint32_t i = 0; i < array.size; i++) { \
                    value = array.data[i]; handle;          \
                }                                           \
        case NK_TYPE_MAP:                                   \
            nk_map_iter_init(obj, &it);                     \
            while (nk_map_iter_has_next(obj, &it)) {        \
                value = it.key; handle;                     \
                value = it.val; handle;                     \
            }                                               \
    }                                                       \
    } while (0)

void nk_gc_init(nk_gc_t* gc) {
    gc->bm_size = 0;
    nk_imap_init(&gc->shared, true, 8);
    gc->heap = gc->bitmap = gc->refmap = NULL;
}

void nk_gc_free(nk_gc_t* gc) {
    nk_gc_collect(gc);
    nk_imap_free(&gc->shared);
    if (gc->heap) {
        NK_FREE(gc->heap);
        NK_FREE(gc->bitmap);
    }
}

static inline bool nk_gc_grow(nk_gc_t* gc) {
    if (NK_UNLIKELY(++gc->bm_size * NK_GC_BITS > ((uint64_t)(UINT32_MAX))))
        return true;

    const size_t map_size = sizeof(uint64_t) * 2 * gc->bm_size;
    const size_t heap_size = sizeof(uintptr_t) * NK_GC_BITS * gc->bm_size;

    if (NK_UNLIKELY(!gc->heap)) {
        gc->heap = (uint64_t*) NK_CALLOC(heap_size, 1);
        gc->bitmap = (uint64_t*) NK_CALLOC(map_size, 1);
    } else {
        gc->heap = (uint64_t*) NK_REALLOC((void*) gc->heap, heap_size);
        gc->bitmap = (uint64_t*) NK_REALLOC((void*) gc->bitmap, map_size);
    }

    gc->refmap = &gc->bitmap[gc->bm_size];   
    return false; 
}

static inline nk_slot_t nk_gc_alloc_slot(nk_gc_t* gc, bool* is_full) {
    int bitpos;
    uint64_t* bitmap;

    if (NK_UNLIKELY(!gc->heap))
        nk_gc_grow(gc);

    bitmap = gc->bitmap;
    for (uint64_t row = 0; row < gc->bm_size; row++) {

        if (!(*bitmap)) {
            *bitmap = 1;
            return row * NK_GC_BITS;
        }

        if (*bitmap == UINT64_MAX) {
            bitmap++;
            continue;
        }

        bitpos = NK_FIRST_BIT(~(*bitmap)) - 1;
        *bitmap |= 1ULL << bitpos;
        return (row * NK_GC_BITS) + bitpos;
    }

    if ((*is_full = nk_gc_collect(gc)))
        return 0;
    return nk_gc_alloc_slot(gc, is_full);
}

#define NK_GC_ALLOC_PROLOG(gc, ret_val)              \
    bool is_full = false;                            \
    nk_slot_t slot = nk_gc_alloc_slot(gc, &is_full); \
    if (is_full) return ret_val

bool nk_gc_alloc_shared(nk_gc_t* gc, nk_value value) {
    NK_GC_ALLOC_PROLOG(gc, false);
    nk_imap_put(&gc->shared, NK_PTR(uint64_t, value), (uint64_t) slot, 0);
    return true;
}

// allocate typed memory on the gc
nk_value nk_gc_alloc(nk_gc_t* gc, uint8_t type, size_t bytes) {
    NK_GC_ALLOC_PROLOG(gc, NK_NULL);

    uint8_t* value_ptr = (uint8_t*) NK_MALLOC(NK_SLOT_SIZE + bytes + 1);
    nk_slot_t* slot_ptr = (nk_slot_t*) (value_ptr + 1);

    *value_ptr = 0;
    gc->heap[(*slot_ptr = slot)] = NK_VALUE(type, (uintptr_t) (slot_ptr + 1));
    return *((nk_value*) &gc->heap[slot]);
}

void nk_gc_share(nk_gc_t* gc, nk_value value) {
    if (nk_is_shared(value))
        return;

    uint8_t* ptr = NK_REAL_PTR(value);
    *((nk_slot_t*) (ptr + 1)) = 1;
    *ptr = 1;

    NK_GC_TRAVERSE(value, nk_gc_share(gc, value));
}

static inline void nk_gc_compact(nk_gc_t* gc) {
    int bitpos;
    uint64_t allocated;
    nk_slot_t slot, move_slot;
    
    move_slot = (gc->bm_size * NK_GC_BITS) - 1;
    for (uint32_t row = 0; row < gc->bm_size; row++) {
        allocated = gc->bitmap[row];
        
        while (allocated) {
            bitpos = NK_FIRST_BIT(allocated) - 1;
            allocated ^= 1ULL << bitpos;
            slot = (row * NK_GC_BITS) + bitpos;
            
            while (move_slot > slot && NK_GC_IS_SET(gc->bitmap, move_slot))
                move_slot--;

            if (move_slot > slot) {
                NK_GC_TOGGLE(gc->bitmap, slot);
                NK_GC_TOGGLE(gc->bitmap, move_slot);
                *NK_SLOT_PTR(*((nk_value*) &gc->heap[slot])) = move_slot;
            }
        }
    }
}

bool nk_gc_collect(nk_gc_t* gc) {
    int bitpos;
    uint8_t* ptr;
    nk_value value;
    uint64_t unreachable;
    bool has_survivors = false;

    for (uint32_t row = 0; row < gc->bm_size; row++) {
        unreachable = gc->bitmap[row] & ~gc->refmap[row];
        gc->bitmap[row] ^= unreachable;

        if (!has_survivors && unreachable)
            has_survivors = true;

        while (unreachable) {
            bitpos = NK_FIRST_BIT(unreachable) - 1;
            unreachable ^= 1ULL << bitpos;
            value = *((nk_value*) &gc->heap[(row * NK_GC_BITS) + bitpos]);
            ptr = NK_REAL_PTR(value);
            
            if (!(*ptr && nk_atomic_sub(((uint32_t*) &ptr[1]), 1, NK_ATOMIC_RELEASE)))
                continue;
            
            switch (NK_TYPE(value)) {
                case NK_TYPE_MAP:
                    nk_imap_free(NK_PTR(nk_imap_t*, value));
                    break;
            }

            if (!nk_is_port(value))
                NK_FREE(ptr);
        }
    }

    if (has_survivors)
        nk_gc_compact(gc);
    return true;
}