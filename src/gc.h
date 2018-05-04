#ifndef _NYANKO_GC_H
#define _NYANKO_GC_H

#include "nk.h"
#include "atomic.h"

typedef struct {
    void** heap;
    bool barrier;
    nk_ring_t grays;
    uint64_t* heap_map;
    uint32_t heap_size;
} nk_gc_t;

void nk_gc_free(nk_gc_t* actor, nk_value value);
nk_value nk_gc_alloc(nk_gc_t* actor, size_t bytes);
nk_value nk_gc_realloc(nk_gc_t* actor, nk_value value, size_t bytes);

void nk_gc_collect(nk_actor_t* actor, bool major);

#endif // _NYANKO_GC_H