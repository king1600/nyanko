#ifndef _NK_ALLOC_H
#define _NK_ALLOC_H

#ifdef NK_USE_MALLOC
    #include <stdlib.h>
    #define NK_FREE free
    #define NK_MALLOC malloc
    #define NK_CALLOC calloc
    #define NK_REALLOC realloc
    #define NK_MEM_FREE
    #define NK_MEM_INIT
    #define NK_MEM_COLLECT
    #define NK_MEM_THREAD_FREE
    #define NK_MEM_THREAD_INIT
#else
    #include <stddef.h>
    #define NK_MALLOC_ATTR  __attribute__((__malloc__))

    void rpmalloc_finalize();
    int rpmalloc_initialize();
    void rpmalloc_thread_collect();
    void rpmalloc_thread_finalize();
    void rpmalloc_thread_initialize();
    void  rpfree(void* ptr);
    void* rprealloc(void* ptr, size_t size);
    void* rpmalloc(size_t size) NK_MALLOC_ATTR;
    void* rpcalloc(size_t n, size_t size) NK_MALLOC_ATTR;

    #define NK_FREE rpfree
    #define NK_MALLOC rpmalloc
    #define NK_CALLOC rpcalloc
    #define NK_REALLOC rprealloc
    #define NK_MEM_FREE rpmalloc_finalize
    #define NK_MEM_INIT rpmalloc_initialize
    #define NK_MEM_THREAD_FREE rpmalloc_thread_finalize
    #define NK_MEM_THREAD_INIT rpmalloc_thread_initialize
#endif

#define NK_CACHE_LOW  0
#define NK_CACHE_MED  1
#define NK_CACHE_HIGH 2
#define NK_THREAD_LOCAL __thread 
#define NK_FIRST_BIT(x) __builtin_ffsll(x)
#define NK_UNREACHABLE() __builtin_unreachable()
#define NK_LIKELY(expr) __builtin_expect(!!(expr), 1)
#define NK_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#define NK_INLINE __attribute__((always_inline)) inline
#define NK_POW2(x) (1ULL << (64ULL - __builtin_clzl((x) - 1ULL)))
#define NK_PREFETCH_READ(addr, locality) __builtin_prefetch(addr, 0, locality)
#define NK_PREFETCH_WRITE(addr, locality) __builtin_prefetch(addr, 1, locality)

#endif // _NK_ALLOC_H