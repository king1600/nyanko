#ifndef _NK_ATOMIC_H
#define _NK_ATOMIC_H

#include <stdint.h>
#include <stdatomic.h>

#define NK_ATOMIC(T) T volatile
#define NK_ALIGN(s) __attribute__((aligned(s)))

#define NK_ATOMIC_RELAXED __ATOMIC_RELAXED
#define NK_ATOMIC_CONSUME __ATOMIC_CONSUME
#define NK_ATOMIC_ACQUIRE __ATOMIC_ACQUIRE
#define NK_ATOMIC_RELEASE __ATOMIC_RELEASE
#define NK_ATOMIC_ACQ_REL __ATOMIC_ACQ_REL
#define NK_ATOMIC_SEQ_CST __ATOMIC_SEQ_CST

#define nk_atomic_fence(mo) __atomic_thread_fence(mo)
#define nk_atomic_load(ptr, mo) __atomic_load_n(ptr, mo)
#define nk_atomic_add(ptr, val, mo) __atomic_fetch_add(ptr, val, mo)
#define nk_atomic_sub(ptr, val, mo) __atomic_fetch_sub(ptr, val, mo)
#define nk_atomic_store(ptr, val, mo) __atomic_store_n(ptr, val, mo)
#define nk_atomic_xchg(ptr, val, mo) __atomic_exchange_n(ptr, val, mo)
#define nk_atomic_cas_weak(ptr, exp, des, succ, fail) __atomic_compare_exchange_n(ptr, exp, des, true, succ, fail)
#define nk_atomic_cas_strong(ptr, exp, des, succ, fail) __atomic_compare_exchange_n(ptr, exp, des, false, succ, fail)

#endif // _NK_ATOMIC_H