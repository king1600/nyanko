#ifndef _NYANKO_H
#define _NYANKO_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(_WIN32)
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
typedef struct { uint32_t size; nk_value* data; } nk_array_t;
typedef struct { uint32_t size; unsigned char* data; } nk_string_t;

nk_value nk_alloc_data(size_t bytes);
nk_value nk_alloc_array(uint32_t size);
nk_value nk_alloc_string(uint32_t bytes);
nk_value nk_alloc_object(nk_value proto);
inline nk_value nk_alloc_int(int32_t v) { return ((nk_value)v) | NK_INT; }
inline nk_value nk_alloc_float(double f) { nk_float_t t = { .f = f }; return t.v; }

void* nk_ptr(nk_value v);
#define nk_int(v) ((int32_t)(v))
bool nk_array(nk_value v, nk_array_t* array);
bool nk_string(nk_value v, nk_string_t* string);
inline double nk_float(nk_value v) { nk_float_t t = { .v = v }; return t.f; }

void  nk_gmark(void* ptr);
void* nk_alloc(size_t bytes);
void  nk_gcollect(bool major);
void* nk_realloc(void* ptr, size_t bytes);

nk_value nk_this();
nk_actor_t* nk_actor_this();
nk_value nk_actor_id(nk_actor_t* actor);
nk_actor_t* nk_actor_spawn(nk_value func, nk_value* args, int n_args);

#endif // _NYANKO_H