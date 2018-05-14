#include "alloc.h"
#include "scheduler.h"

static inline nk_mpmc_qnode_t* nk_mpmc_node_alloc(void* data) {
    nk_mpmc_qnode_t* node = (nk_mpmc_qnode_t*) NK_MALLOC(sizeof(nk_mpmc_qnode_t));
    node->next = NULL;
    node->data = data;
    return node;
}

static NK_INLINE bool nk_atomic_cas_aba(nk_mpmc_qnode_aba_t* ptr, nk_mpmc_qnode_aba_t* exp, nk_mpmc_qnode_aba_t des) {
    #ifdef NK_WINDOWS
    return _InterlockedCompareExchange128((LONGLONG*)ptr, (LONGLONG)des.count, (LONGLONG)des.node, (LONGLONG*)exp);
    #else
    return nk_atomic_cas_weak(&ptr->raw, &exp->raw, des.raw, NK_ATOMIC_RELAXED, NK_ATOMIC_RELAXED);
    #endif
}

void nk_mpmc_queue_init(nk_mpmc_queue_t* queue) {
    queue->head = nk_mpmc_node_alloc(NULL);
    queue->tail.node = queue->head;
    queue->tail.count = 0;
}

void nk_mpmc_queue_free(nk_mpmc_queue_t* queue) {
    nk_atomic_store(&queue->head, NULL, NK_ATOMIC_RELAXED);
    NK_FREE((void*) queue->tail.node);
    queue->tail.node = NULL;
}

void nk_mpmc_queue_push(nk_mpmc_queue_t* queue, void* data) {
    nk_mpmc_qnode_t* node = nk_mpmc_node_alloc(data);
    nk_atomic_fence(NK_ATOMIC_RELEASE);

    nk_mpmc_qnode_t* prev = nk_atomic_xchg(&queue->head, node, NK_ATOMIC_RELAXED);
    nk_atomic_store(&prev->next, node, NK_ATOMIC_RELAXED);
}

void* nk_mpmc_queue_pop(nk_mpmc_queue_t* queue) {
    nk_mpmc_qnode_t *tail, *next;
    nk_mpmc_qnode_aba_t cmp, xchg;
    cmp.node = queue->tail.node;
    cmp.count = queue->tail.count;

    do {
        tail = cmp.node;
        next = nk_atomic_load(&tail->next, NK_ATOMIC_RELAXED);
        if (next == NULL)
            return NULL;

        xchg.node = next;
        xchg.count = cmp.count + 1;
    } while (!nk_atomic_cas_aba(&queue->tail, &cmp, xchg));

    nk_atomic_fence(NK_ATOMIC_ACQ_REL);
    void* data = nk_atomic_load(&next->data, NK_ATOMIC_RELAXED);

    nk_atomic_store(&next->data , NULL, NK_ATOMIC_RELAXED);
    while (nk_atomic_load(&tail->data, NK_ATOMIC_RELAXED) != NULL)
        nk_sched_yield();

    nk_atomic_fence(NK_ATOMIC_ACQUIRE);
    NK_FREE((void*) tail);
    return data;
}
