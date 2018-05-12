#ifndef _NK_MAP_H
#define _NK_MAP_H

#include "nk.h"

typedef struct {
    void** cells;
    uint32_t size;
    uint32_t capacity;
    bool is_chained;
} nk_imap_t;

void nk_imap_free(nk_imap_t* map);

void nk_imap_init(nk_imap_t* map, bool is_chained, uint32_t init_cap);

uint64_t nk_imap_get(nk_imap_t* map, uint64_t key, uint64_t none);

uint64_t nk_imap_drop(nk_imap_t* map, uint64_t key, uint64_t none);

uint64_t nk_imap_put(nk_imap_t* map, uint64_t key, uint64_t value, uint64_t none);


#endif // _NK_MAP_H