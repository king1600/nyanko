#include "nk.h"
#include "rpmalloc.h"

typedef struct {
    nk_value key;
    nk_value value;
} nk_obj_cell;

typedef struct {
    size_t size;
    size_t capacity;
    nk_obj_cell* cells;
} nk_object_t;

nk_value nk_alloc_object(nk_value proto) {
    return NK_NULL;
}