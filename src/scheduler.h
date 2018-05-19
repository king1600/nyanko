#ifndef _NK_SCHEDULER_H
#define _NK_SCHEDULER_H

#include "actor.h"

#ifdef NK_WINDOWS
typedef __int128_t nk_aba_t;
#else
typedef int64_t nk_aba_t;
#endif

typedef struct nk_mpmc_qnode_t {
    NK_ATOMIC(void*) data;
    NK_ATOMIC(struct nk_mpmc_qnode_t*) next;
} nk_mpmc_qnode_t;

typedef union {
    struct { nk_mpmc_qnode_t* node; uintptr_t count; };
    nk_aba_t raw;
} nk_mpmc_qnode_aba_t;

typedef struct {
    NK_ALIGN(64) NK_ATOMIC(nk_mpmc_qnode_t*) head;
    NK_ALIGN(sizeof(nk_aba_t) << 3) nk_mpmc_qnode_aba_t tail;
} nk_mpmc_queue_t;

void nk_mpmc_queue_init(nk_mpmc_queue_t* queue);

void nk_mpmc_queue_free(nk_mpmc_queue_t* queue);

void* nk_mpmc_queue_pop(nk_mpmc_queue_t* queue);

void nk_mpmc_queue_push(nk_mpmc_queue_t* queue, void* data);

//////////////////////////////////////////////////////////////////////

typedef struct nk_sched_t nk_sched_t;
typedef struct {
    size_t size;
    nk_sched_t* workers;
} nk_schedlist_t;

void nk_sched_init(nk_vm_t* vm, nk_schedlist_t* list, size_t amount);

void nk_sched_free(nk_schedlist_t* list);

void nk_sched_submit(nk_actor_t* actor);

#endif // _NK_SCHEDULER_H