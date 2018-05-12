#ifndef _NK_GC_H
#define _NK_GC_H

#include "nk.h"
#include "map.h"

typedef struct {
    uint64_t* heap;
    uint64_t* bitmap;
    uint64_t* refmap;
    uint32_t bm_size;
    nk_imap_t shared;
} nk_gc_t;

void nk_gc_init(nk_actor_t* actor);

void nk_gc_free(nk_actor_t* actor);

bool nk_gc_collect(nk_actor_t* actor);

void nk_gc_share(nk_actor_t* actor, nk_value value);

bool nk_gc_alloc_shared(nk_actor_t* actor, nk_value value);

nk_value nk_gc_alloc(nk_actor_t* actor, uint8_t type, size_t bytes);

#endif // _NK_GC_H