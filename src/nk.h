#ifndef _NK_H
#define _NK_H

// platform detection
#if defined(_WIN32) || defined(_WIN64)
#define NK_WINDOWS
#elif defined(__linux__)
#define NK_LINUX
#else
#error "Platform not supported"
#endif

// includes
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// values structures
typedef double nk_value;
typedef uint32_t nk_slot_t;
typedef struct nk_actor_t nk_actor_t;
typedef struct { uint32_t size; uint8_t* data; } nk_string_t;
typedef struct { uint32_t size; nk_value* data; } nk_array_t;
typedef struct { uint32_t index; nk_value key; nk_value val; void* ptr; } nk_map_iter_t;

// constants
#define NK_TYPE_NULL   0
#define NK_TYPE_INT    1
#define NK_TYPE_MAP    2
#define NK_TYPE_FUNC   3
#define NK_TYPE_PORT   4
#define NK_TYPE_DATA   5
#define NK_TYPE_ARRAY  6
#define NK_TYPE_STRING 7
#define NK_NULL NK_VALUE(NK_TYPE_NULL, 0)
#define NK_TRUE NK_VALUE(NK_TYPE_INT, 1)
#define NK_FALSE NK_VALUE(NK_TYPE_INT, 0)

// internal conversions
#define NK_INT(v) (*((int32_t*) &(v)))
#define NK_FROM(v) (*((nk_value*) &(v)))
#define NK_UPTR(v) (*((uintptr_t*) &(v)))
#define NK_TO_64(v) (*((uint64_t*) &(v)))
#define NK_TYPE(v) ((((uint8_t*) &(v))[6]) & 0b111)
#define NK_PTR(type, v) ((type)(NK_UPTR(v) & 0xffffffffffffULL))
#define NK_SLOT_PTR(v) (NK_PTR(uint8_t*, v) - sizeof(nk_slot_t))
#define NK_REAL_PTR(v) (NK_PTR(uint8_t*, v) - sizeof(nk_slot_t) - sizeof(uint8_t))
static inline nk_value NK_VALUE(int type, uintptr_t data) {
    uint64_t value = ((0xfff8ULL | type) << 48) | data;
    return NK_FROM(value);
}

// type checking
#define nk_is_int(v) (NK_TYPE(v) == NK_TYPE_INT)
#define nk_is_map(v) (NK_TYPE(v) == NK_TYPE_MAP)
#define nk_is_func(v) (NK_TYPE(v) == NK_TYPE_FUNC)
#define nk_is_data(v) (NK_TYPE(v) == NK_TYPE_DATA)
#define nk_is_port(v) (NK_TYPE(v) == NK_TYPE_PORT)
#define nk_is_null(v) (NK_TYPE(v) == NK_TYPE_NULL)
#define nk_is_array(v) (NK_TYPE(v) == NK_TYPE_ARRAY)
#define nk_is_string(v) (NK_TYPE(v) == NK_TYPE_STRING)
#define nk_is_float(v) (NK_UPTR(v) < (0xfff8ULL << 48))
#define nk_is_ptr(v) (!(nk_is_float(v) || nk_is_int(v)))
#define nk_is_shared(v) (nk_is_ptr(v) && *NK_REAL_PTR(v))

// allocation
nk_value nk_alloc_data(size_t bytes);
nk_value nk_alloc_map(bool is_chained);
nk_value nk_alloc_array(uint32_t items);
nk_value nk_alloc_string(uint32_t bytes);
inline nk_value nk_alloc_float(double v) { return v; }
inline nk_value nk_alloc_int(int32_t v) { return NK_VALUE(NK_TYPE_INT, v); }

// data interaction
bool nk_get_array(nk_value value, nk_array_t* array);
bool nk_get_string(nk_value value, nk_string_t* string);
nk_value nk_map_get(nk_value map, nk_value key);
nk_value nk_map_drop(nk_value map, nk_value key);
void nk_map_iter_init(nk_value map, nk_map_iter_t* it);
bool nk_map_iter_has_next(nk_value map, nk_map_iter_t* it);
nk_value nk_map_put(nk_value map, nk_value key, nk_value val);
inline double nk_get_float(nk_value value) { return value; }
inline int32_t nk_get_int(nk_value value) { return NK_INT(value); }

// misc.
void nk_gcollect();
nk_value nk_this();
uint32_t nk_hash(nk_value value);
void nk_send(nk_value actor_id, nk_value value);

#endif // _NK_H