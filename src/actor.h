#ifndef _NK_ACTOR_H
#define _NK_ACTOR_H

#include "gc.h"
#include "atomic.h"

typedef struct {
    nk_value* bp;
    nk_value* sp;
    nk_value* stack;
    size_t stack_size;
    nk_value x, y, z;
} nk_frame_t;

typedef struct nk_msgq_node_t nk_msgq_node_t;
typedef struct {
    NK_ATOMIC(nk_msgq_node_t*) head;
    NK_ATOMIC(nk_msgq_node_t*) tail;
} nk_msgq_t;

typedef struct {
    nk_gc_t gc;
    nk_frame_t frame;
    nk_msgq_t mailbox;
} nk_actor_t;

void nk_actor_init(nk_actor_t* actor);

void nk_actor_free(nk_actor_t* actor);

nk_value nk_actor_recv(nk_actor_t* actor);

void nk_actor_send(nk_actor_t* actor, nk_value value);

#endif // _NK_ACTOR_H