#include "vm.h"
#include "scheduler.h"
#include <string.h>

NK_THREAD_LOCAL nk_actor_t* __nk_actor = NULL;

nk_actor_t* nk_actor_this(nk_actor_t* set) {
    if (set != NULL)
        __nk_actor = set;
    return __nk_actor;
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

void nk_actor_free(nk_actor_t* actor) {
    if (nk_atomic_sub(&actor->refc, 1, NK_ATOMIC_RELEASE) == 1) {
        nk_gc_free(actor);
        NK_FREE(actor);
        for (nk_value _ = nk_actor_recv(actor); !nk_is_null(_); _ = nk_actor_recv(actor))
            nk_thread_yield();
    }
}

nk_actor_t* nk_actor_spawn(nk_func_t* func, uint8_t argc, nk_value* argv) {
    nk_actor_t* actor = (nk_actor_t*) NK_MALLOC(sizeof(nk_actor_t));
    nk_frame_t* frame = &actor->frame;

    actor->refc = 1;
    actor->active = 0;
    actor->mailbox.head = actor->mailbox.tail = nk_msgq_node(NK_NULL);

    nk_gc_init(actor);
    
    frame->ip.u8 = func->code;
    frame->tp = frame->rp = NULL;
    uint8_t max_args = NK_MAX(func->mx_args, argc);
    frame->stack_size = max_args + func->num_locals + func->mx_stack;
    frame->stack = (nk_value*) NK_MALLOC(sizeof(nk_value) * frame->stack_size);
    frame->sp = &(frame->bp = &frame->stack[max_args])[func->num_locals];

    memcpy((void*) frame->stack, (const void*) argv, func->mx_args);
    for (uint8_t empty = max_args - func->mx_args; empty != 0;)
        frame->stack[empty--] = NK_NULL;

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