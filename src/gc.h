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

void nk_gc_init(nk_gc_t* gc);

void nk_gc_free(nk_gc_t* gc);

bool nk_gc_collect(nk_gc_t* gc);

void nk_gc_share(nk_gc_t* gc, nk_value value);

bool nk_gc_alloc_shared(nk_gc_t* gc, nk_value value);

nk_value nk_gc_alloc(nk_gc_t* gc, uint8_t type, size_t bytes);

#endif // _NK_GC_H