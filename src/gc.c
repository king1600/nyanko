#include "actor.h"
#include "rpmalloc.h"
#include <string.h>

/*
typedef struct {
    void** heap;
    void** cells;
    bool barrier;
    nk_ring_t grays;
    uint64_t* hp_map;
    uint64_t* cl_map;
    uint32_t hp_size;
    uint32_t cl_size;
} nk_gc_t;
*/

#define NK_GC_WHITE 0
#define NK_GC_BLACK 1
#define NK_PTR_SHIFT 16
#define NK_PTR_BITS 0x0040
#define NK_GC_BITS (sizeof(uint64_t) << 3)

static inline uint64_t nk_gc_alloc_slot(nk_gc_t* gc, void* ptr) {
    int bitpos;
    uint64_t open_slot;
    uint64_t* bitmap = gc->heap_map;

    if (NK_UNLIKELY(gc->heap_map == NULL)) {
        bitmap = gc->heap_map = (uint64_t*) rpcalloc(32, 1);
        gc->heap = rpmalloc(sizeof(void*) * NK_GC_BITS * gc->heap_size);
    }

    for (uint64_t slot = 0; slot < gc->heap_size; slot++) {
        NK_PREFETCH_READ(&bitmap[1], 1);
        NK_PREFETCH_WRITE(&bitmap[1], 1);

        if (*bitmap == 0) {
            *bitmap = 1;
            gc->heap[open_slot = slot * NK_GC_BITS] = ptr;
            return open_slot;
        }

        if (*bitmap == ((uint64_t)-1)) {
            bitmap++;
            continue;
        }

        bitpos = NK_FIRST_BIT(~(*bitmap)) - 1;
        *bitmap |= 1ULL << bitpos;
        gc->heap[open_slot = slot * NK_GC_BITS * bitpos] = ptr;
        return open_slot;
    }

    size_t new_size = sizeof(uint64_t) * gc->heap_size * 2;
    gc->heap_map = (uint64_t*) rprealloc(gc->heap_map, new_size);
    gc->heap = rprealloc(gc->heap, sizeof(void*) * NK_GC_BITS * gc->heap_size);
    memset((void*)(gc->heap_map + (gc->heap_size - 1)), NK_GC_WHITE, sizeof(uint64_t));

    gc->heap_map[gc->heap_size - 1] = 1;
    gc->heap[open_slot = (gc->heap_size - 1) * NK_GC_BITS] = ptr;
    return open_slot;
}

static NK_INLINE void nk_gc_mark(nk_gc_t* gc, nk_value value) {
    // TODO
}

static inline void nk_gc_mark_roots(nk_gc_t* gc, nk_frame_t* frame) {
    // TODO: Mark all registers and scan the stack
    gc->barrier = true;
}

static inline void nk_gc_sweep(nk_gc_t* gc) {
    // TODO
}

void* nk_ptr(nk_value value) {
    uintptr_t ptr_val = ((value & 0xffffffffffffULL) << NK_PTR_SHIFT);
    return (void*) (ptr_val | NK_PTR_BITS);
}

nk_value nk_from_ptr(void* ptr, nk_value type) {
    const uintptr_t ptr_val = (uintptr_t) ptr;
    return (type & (0xffffULL << 48)) | (ptr_val >> NK_PTR_SHIFT);
}

nk_value nk_alloc(size_t bytes) { return nk_gc_alloc(&nk_actor_this()->gc, bytes); }
nk_value nk_gc_alloc(nk_gc_t* gc, size_t bytes) {
    uint64_t* real_ptr = (uint64_t*) rpmalloc(bytes + sizeof(uint64_t));
    void* ptr = (void*) (real_ptr + 1);
    *real_ptr = nk_gc_alloc_slot(gc, ptr);
    return nk_from_ptr(ptr, NK_DATA);
}

nk_value nk_realloc(nk_value value, size_t bytes) { return nk_gc_realloc(&nk_actor_this()->gc, value, bytes); }
nk_value nk_gc_realloc(nk_gc_t* gc, nk_value value, size_t bytes) {
    bytes = bytes < sizeof(uint64_t) ? sizeof(uint64_t) : bytes;
    void* real_ptr = (void*) (((uint64_t*) nk_ptr(value)) - 1);
    real_ptr = rprealloc(real_ptr, bytes);
    return nk_from_ptr(real_ptr, value);
}

void nk_gcollect(bool major) { nk_gc_collect(nk_actor_this(), major); }
void nk_gc_collect(nk_actor_t* actor, bool major) {
    nk_value gray;
    nk_gc_t* gc = &actor->gc;

    if (major) {
        if (!gc->barrier)
            nk_gc_mark_roots(gc, &actor->frame);
        while ((gray = (nk_value) nk_ring_pop(&gc->grays)) != 0)
            nk_gc_mark(gc, gray);
        nk_gc_sweep(gc);

    } else {
        if (!gc->barrier)
            nk_gc_mark_roots(gc, &actor->frame);
        else if ((gray = (nk_value) nk_ring_pop(&gc->grays)) != 0)
            nk_gc_mark(gc, gray);
        else
            nk_gc_sweep(gc);
    }
}

void nk_free(nk_value value) { nk_gc_free(&nk_actor_this()->gc, value); }
void nk_gc_free(nk_gc_t* gc, nk_value value) {
    if (!nk_is_ptr(value)) return;

    uint64_t* real_ptr = ((uint64_t*) nk_ptr(value)) - 1;
    uint64_t bitpos = 1ULL << ((*real_ptr) & (NK_GC_BITS - 1));
    gc->heap_map[*real_ptr / NK_GC_BITS] &= ~bitpos;

    if (nk_is_object(value)) {
        nk_obj_t* obj = (nk_obj_t*)(real_ptr + 1);
        for (size_t i = 0; i < obj->capacity; i++)
            if (((uintptr_t)(&obj->cells[i])) > 1)
                rpfree((void*) &obj->cells[i]);
    }

    // TODO: add finalization support

    rpfree((void*) real_ptr);
}