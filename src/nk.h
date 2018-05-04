#ifndef _NYANKO_H
#define _NYANKO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(_WIN32) || defined(_WIN64)
#define NK_WINDOWS
#elif defined(__linux__)
#define NK_LINUX
#endif

#define NK_FALSE  (NK_INT | 0)
#define NK_TRUE   (NK_INT | 1)
#define NK_NULL   (0xfff8ULL << 48)
#define NK_INT    ((0xfff0ULL | 0) << 48)
#define NK_DATA   ((0xfff0ULL | 1) << 48)
#define NK_FUNC   ((0xfff0ULL | 2) << 48)
#define NK_ARRAY  ((0xfff0ULL | 3) << 48)
#define NK_STRING ((0xfff0ULL | 4) << 48)
#define NK_OBJECT ((0xfff0ULL | 5) << 48)

#define NK_TYPE(v) ((v) & (0b111ULL << 48))
#define nk_is_null(v) ((v) == NK_NULL)
#define nk_is_float(v) ((v) < NK_NULL)
#define nk_is_int(v) (NK_TYPE(v) == NK_INT)
#define nk_is_data(v) (NK_TYPE(v) == NK_DATA)
#define nk_is_func(v) (NK_TYPE(v) == NK_FUNC)
#define nk_is_array(v) (NK_TYPE(v) == NK_ARRAY)
#define nk_is_string(v) (NK_TYPE(v) == NK_STRING)
#define nk_is_object(v) (NK_TYPE(v) == NK_OBJECT)
#define nk_is_ptr(v) (!(nk_is_float(v) || nk_is_int(v) || nk_is_null(v)))

typedef uint64_t nk_value;
typedef struct nk_actor_t nk_actor_t;
typedef union { double f; nk_value v; } nk_float_t;
typedef struct { nk_value key; nk_value value; } nk_cell_t;
typedef struct { uint32_t size; nk_value* data; } nk_array_t;
typedef struct { uint32_t size; unsigned char* data; } nk_string_t;
typedef struct { size_t size; size_t capacity; nk_cell_t* cells; } nk_obj_t;

#define nk_alloc_data nk_alloc
nk_value nk_alloc_array(uint32_t size);
nk_value nk_alloc_string(uint32_t bytes);
nk_value nk_alloc_object(nk_value proto);
inline nk_value nk_alloc_int(int32_t v) { return ((nk_value)v) | NK_INT; }
inline nk_value nk_alloc_float(double f) { nk_float_t t = { .f = f }; return t.v; }

void* nk_ptr(nk_value v);
#define nk_int(v) ((int32_t)(v))
nk_value nk_from_ptr(void* ptr, nk_value type);
bool nk_array(nk_value v, nk_array_t* array);
bool nk_string(nk_value v, nk_string_t* string);
inline double nk_float(nk_value v) { nk_float_t t = { .v = v }; return t.f; }

void nk_free(nk_value value);
void  nk_gcollect(bool major);
nk_value nk_alloc(size_t bytes);
nk_value nk_realloc(nk_value value, size_t bytes);

nk_value nk_this();
nk_actor_t* nk_actor_this();
void nk_actor_free(nk_actor_t* actor);
nk_value nk_actor_id(nk_actor_t* actor);
bool nk_actor_send(nk_actor_t* actor, nk_value value);
nk_actor_t* nk_actor_spawn(nk_value func, nk_value* args, int n_args);

#endif // _NYANKO_H