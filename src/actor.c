#include "alloc.h"
#include "scheduler.h"

NK_THREAD_LOCAL nk_actor_t* __nk_actor = NULL;

nk_actor_t* nk_actor_this(nk_actor_t* set) {
    if (set != NULL)
        __nk_actor = set;
    return __nk_actor;
}

void nk_actor_free(nk_actor_t* actor) {
    if (nk_atomic_sub(&actor->refc, 1, NK_ATOMIC_RELEASE) == 1) {
        nk_gc_free(actor);
        NK_FREE(actor);
        for (nk_value _ = nk_actor_recv(actor); !nk_is_null(_); _ = nk_actor_recv(actor))
            nk_sched_yield();
    }
}

struct nk_msgq_node_t {
    nk_value value;
    NK_ATOMIC(nk_msgq_node_t*) next;
};

static inline nk_msgq_node_t* nk_msgq_node(nk_value value) {
    nk_msgq_node_t* node = (nk_msgq_node_t*) NK_MALLOC(sizeof(nk_msgq_node_t));
    node->next = NULL;
    node->value = value;
    return node;
}

nk_actor_t* nk_actor_spawn() {
    nk_actor_t* actor = (nk_actor_t*) NK_MALLOC(sizeof(nk_actor_t));

    actor->refc = 1;
    actor->active = 0;
    actor->mailbox.head = actor->mailbox.tail = nk_msgq_node(NK_NULL);

    nk_gc_init(actor);
    
    return actor;
}

void nk_actor_send(nk_actor_t* actor, nk_value value) {
    nk_msgq_node_t* node = nk_msgq_node(value);
    nk_msgq_node_t* prev = nk_atomic_xchg(&actor->mailbox.head, node, NK_ATOMIC_ACQ_REL);
    nk_atomic_store(&prev->next, node, NK_ATOMIC_RELEASE);

    uint8_t is_idle = 0;
    if (nk_atomic_cas_weak(&actor->active, &is_idle, 1, NK_ATOMIC_RELAXED, NK_ATOMIC_RELAXED))
        nk_sched_submit(actor);
}

nk_value nk_actor_recv(nk_actor_t* actor) {
    nk_msgq_node_t* tail = actor->mailbox.tail;
    nk_msgq_node_t* next = nk_atomic_load(&tail->next, NK_ATOMIC_ACQUIRE);

    if (next == NULL)
        return NK_NULL;

    actor->mailbox.tail = next;
    nk_value value = next->value;
    nk_atomic_fence(NK_ATOMIC_ACQUIRE);
    NK_FREE((void*) tail);

    return value;
}