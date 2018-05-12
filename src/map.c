#include "map.h"
#include "alloc.h"

static inline uint32_t nk_hash_int(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    return (x >> 16) ^ x;
}

///////////////////// Hashmap internals ////////////////////////

typedef struct {
    uint64_t key;
    uint64_t value;
} nk_cell_t;

typedef struct nk_cell_chain_t {
    uintptr_t key;
    uint32_t value;
    struct nk_cell_chain_t* next;
} nk_cell_chain_t;

#define NK_IMAP_RESIZE_PROLOG(map)                                                      \
    uint32_t old_capacity = (map)->capacity;                                            \
    nk_cell_chain_t** old_cells = (nk_cell_chain_t**) (map)->cells;                     \
    (map)->cells = (void**) NK_CALLOC((map)->capacity <<= 1, sizeof(nk_cell_chain_t*)); \
    if ((map)->capacity < old_capacity) {                                               \
        NK_FREE(old_cells);                                                             \
        return;                                                                         \
    }

/////////////////////  Chained Hashmap internals ////////////////////////

#define NK_IMAP_FIND_CHAINED(map, key, on_none)             \
    uint32_t slot;                                          \
    nk_cell_chain_t *last, *cell;                           \
    if (NK_UNLIKELY(!(map)->cells))                         \
        on_none;                                            \
    cell = nk_imap_find_chained((map), (key), &slot, &last)

#define NK_IMAP_ITER_CHAINED(cells, capacity, on_cell) do {           \
    nk_cell_chain_t *cell, *next;                                     \
    for (uint32_t i = 0; i < capacity; i++) {                         \
        for (cell = ((nk_cell_chain_t**)(cells))[i]; cell != NULL;) { \
            next = cell->next;                                        \
            on_cell;                                                  \
            cell = next;                                              \
        }                                                             \
    }                                                                 \
} while (0)

static nk_cell_chain_t* nk_imap_find_chained(nk_imap_t* map, uint64_t key, uint32_t* slot, nk_cell_chain_t** last) {
    nk_cell_chain_t* cell;
    *slot = nk_hash_int((uint32_t) key) & (map->capacity - 1);

    *last = NULL;
    if (!(cell = ((nk_cell_chain_t**) map->cells)[*slot]))
        return cell;

    while (cell != NULL) {
        if (cell->key == key)
            return cell;
        if (*last == NULL)
            *last = cell;
        cell = cell->next;
    }

    return NULL;
}

static uint64_t nk_imap_put_chained(nk_imap_t* map, uint64_t key, uint64_t value, uint64_t none);
static inline void nk_imap_resize_chained(nk_imap_t* map) {
    NK_IMAP_RESIZE_PROLOG(map)

    map->size = 0;
    NK_IMAP_ITER_CHAINED(old_cells, old_capacity,
        nk_imap_put_chained(map, cell->key, cell->value, 0);
        NK_FREE(cell)
    );

    NK_FREE(old_cells);
}

static inline uint64_t nk_imap_get_chained(nk_imap_t* map, uint64_t key, uint64_t none) {
    NK_IMAP_FIND_CHAINED(map, key, return none);
    return cell != NULL ? ((uint64_t) cell->value) : none;
}

static inline uint64_t nk_imap_drop_chained(nk_imap_t* map, uint64_t key, uint64_t none) {
    NK_IMAP_FIND_CHAINED(map, key, return none);
    if (cell == NULL)
        return none;

    if (last != NULL)
        last->next = cell->next;
    else
        ((nk_cell_chain_t**) map->cells)[slot] = NULL;

    uint64_t old_value = (uint64_t) cell->value;
    NK_FREE(cell);
    return old_value;
}

static uint64_t nk_imap_put_chained(nk_imap_t* map, uint64_t key, uint64_t value, uint64_t none) {
    NK_IMAP_FIND_CHAINED(map, key,
        map->cells = (void**) NK_CALLOC(map->capacity, sizeof(nk_cell_chain_t*)));

    if (cell != NULL) {
        uint64_t old_value = (uint64_t) cell->value;
        cell->value = value;
        return old_value;
    }

    cell = NK_MALLOC(sizeof(nk_cell_chain_t));
    cell->key = key;
    cell->next = NULL;
    cell->value = value;
    
    if (last != NULL)
        last->next = cell;
    else
        ((nk_cell_chain_t**) map->cells)[slot] = cell;

    if (((uint64_t)(++map->size) * 4) > ((uint64_t)(map->capacity) * 3))
        nk_imap_resize_chained(map);
    return none;
}

///////////////////// Probing Hashmap internals ////////////////////////

#define NK_PROBE_CELL_DELETED ((nk_cell_t*) 1)
#define NK_IMAP_FIND_PROBED(map, key, on_none)      \
    uint32_t slot;                                  \
    nk_cell_t* cell;                                \
    if (NK_UNLIKELY(!(map)->cells))                 \
        on_none;                                    \
    slot = nk_imap_find_probed((map), (key), &cell) 

#define NK_IMAP_ITER_PROBED(cells, capacity, on_cell)                      \
    nk_cell_t* cell;                                                       \
    for (uint32_t i = 0; i < (capacity); i++) {                            \
        if ((cell = ((nk_cell_t**) (cells))[i]) > NK_PROBE_CELL_DELETED) { \
            on_cell;                                                       \
        }                                                                  \
    }
    

static inline uint32_t nk_imap_find_probed(nk_imap_t* map, uint64_t key, nk_cell_t** cell) {
    uint32_t index;
    const uint32_t hash = nk_hash_int(key);
    const uint32_t mask = map->capacity - 1;
    nk_cell_t** cells = (nk_cell_t**) map->cells;

    index = hash & mask;
    *cell = cells[index];
    for (uint32_t probe = 1; probe <= map->capacity; probe++) {
        if (*cell != NK_PROBE_CELL_DELETED) 
            break;
        index = (hash + (probe * probe)) & mask;
        *cell = cells[index];
    }

    return index;
}

static uint64_t nk_imap_put_probed(nk_imap_t* map, uint64_t key, uint64_t value, uint64_t none);
static inline void nk_imap_resize_probed(nk_imap_t* map) {
    NK_IMAP_RESIZE_PROLOG(map)

    map->size = 0;
    NK_IMAP_ITER_PROBED(old_cells, old_capacity,
        nk_imap_put_probed(map, cell->key, cell->value, 0);
        NK_FREE(cell)
    );

    NK_FREE(old_cells);
}

static inline uint64_t nk_imap_get_probed(nk_imap_t* map, uint64_t key, uint64_t none) {
    NK_IMAP_FIND_PROBED(map, key, return none);
    return cell != NULL ? ((uint64_t) cell->value) : none;
}

static inline uint64_t nk_imap_drop_probed(nk_imap_t* map, uint64_t key, uint64_t none) {
    uint64_t old_value;
    NK_IMAP_FIND_PROBED(map, key, return none);
    if (cell == NULL)
        return none;

    old_value = (uint64_t) cell->value;
    ((nk_cell_t**) map->cells)[slot] = NK_PROBE_CELL_DELETED;
    NK_FREE(cell);
    return old_value;
}

static inline uint64_t nk_imap_put_probed(nk_imap_t* map, uint64_t key, uint64_t value, uint64_t none) {
    NK_IMAP_FIND_PROBED(map, key,
        map->cells = (void**) NK_CALLOC(map->capacity, sizeof(nk_cell_t*)));

    if (cell != NULL) {
        uint64_t old_value = cell->value;
        cell->value = value;
        return old_value;
    }

    cell = (nk_cell_t*) NK_MALLOC(sizeof(nk_cell_t));
    cell->key = key;
    cell->value = value;
    ((nk_cell_t**) map->cells)[slot] = cell;

    if (((uint64_t)(++map->size) * 4) > ((uint64_t)(map->capacity) * 3))
        nk_imap_resize_probed(map);
    return none;
}

////////////////// External Functions //////////////////////////////////

void nk_imap_init(nk_imap_t* map, bool is_chained, uint32_t init_cap) {
    map->size = 0;
    map->cells = NULL;
    map->is_chained = is_chained;
    map->capacity = NK_POW2(init_cap);
}

uint64_t nk_imap_get(nk_imap_t* map, uint64_t key, uint64_t none) {
    return map->is_chained ?
        nk_imap_get_chained(map, key, none) :
        nk_imap_get_probed(map, key, none);
}

uint64_t nk_imap_drop(nk_imap_t* map, uint64_t key, uint64_t none) {
    return map->is_chained ?
        nk_imap_drop_chained(map, key, none) :
        nk_imap_drop_probed(map, key, none);
}

uint64_t nk_imap_put(nk_imap_t* map, uint64_t key, uint64_t value, uint64_t none) {
    return map->is_chained ?
        nk_imap_put_chained(map, key, value, none) :
        nk_imap_put_probed(map, key, value, none);
}

void nk_imap_free(nk_imap_t* map) {
    if (NK_UNLIKELY(!map->cells))
        return;

    if (map->is_chained) {
        NK_IMAP_ITER_CHAINED(map->cells, map->capacity, NK_FREE(cell));
    } else {
        NK_IMAP_ITER_PROBED(map->cells, map->capacity, NK_FREE(cell));
    }

    NK_FREE(map->cells);
}