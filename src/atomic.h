#ifndef _NYANKO_ATOMIC_H
#define _NYANKO_ATOMIC_H

#define NK_UNREACHABLE() __builtin_unreachable()
#define NK_LIKELY(expr) __builtin_expect(!!(expr), 1)
#define NK_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#define NK_INLINE __attribute__((always_inline)) inline

#include <stdatomic.h>
#define NK_ATOMIC(type) volatile type
#define NK_ATOMIC_INIT(ptr, val) atomic_init(ptr, val)

#define nk_atomic_relaxed memory_order_relaxed
#define nk_atomic_consume memory_order_consume
#define nk_atomic_acquire memory_order_acquire
#define nk_atomic_release memory_order_release
#define nk_atomic_seq_cst memory_order_seq_cst

#define nk_atomic_fence(order) atomic_thread_fence(order)
#define nk_atomic_load(ptr, order) atomic_load_explicit(ptr, order)
#define nk_atomic_store(ptr, order) atomic_store_explicit(ptr, order)
#define nk_atomic_swap(ptr, swap, order) atomic_exchange_explicit(ptr, swap, order)
#define nk_atomic_add(ptr, value, order) atomic_fetch_add_explicit(ptr, value, order)
#define nk_atomic_sub(ptr, value, order) atomic_fetch_sub_explicit(ptr, value, order)
#define nk_atomic_cas(ptr, test, swap, succ, fail) atomic_compare_exchange_explicit(ptr, test, swap, succ, fail)
#define nk_atomic_cas_weak(ptr, test, swap, succ, fail) atomic_compare_exchange_weak_explicit(ptr, test, swap, succ, fail)
#define nk_atomic_cas_strong(ptr, test, swap, succ, fail) atomic_compare_exchange_strong_explicit(ptr, test, swap, succ, fail)

typedef struct {
    size_t size;
    size_t rear;
    size_t front;
    void** data;
} nk_ring_t;

void nk_ring_init(nk_ring_t* ring, size_t size);
bool nk_ring_push(nk_ring_t* ring, void* value);
void* nk_ring_pop(nk_ring_t* ring);

typedef struct nk_qnode_t nk_qnode_t;
typedef struct {
    NK_ATOMIC(nk_qnode_t*) head;
    NK_ATOMIC(nk_qnode_t*) tail;
} nk_queue_t;

void nk_queue_init(nk_queue_t* queue);
void nk_queue_free(nk_queue_t* queue);
void* nk_queue_pop(nk_queue_t* queue);
void nk_queue_push(nk_queue_t* queue, void* value);

#endif // _NYANKO_ATOMIC_H