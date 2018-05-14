#ifndef _NK_ACTOR_H
#define _NK_ACTOR_H

#include "gc.h"
#include "atomic.h"

#define NK_NUM_REGISTERS 8

typedef struct {
    nk_value* bp;
    nk_value* sp;
    nk_value* stack;
    size_t stack_size;
    nk_value registers[NK_NUM_REGISTERS]; 
} nk_frame_t;

typedef struct nk_msgq_node_t nk_msgq_node_t;
typedef struct {
    NK_ATOMIC(nk_msgq_node_t*) head;
    NK_ATOMIC(nk_msgq_node_t*) tail;
} nk_msgq_t;

struct nk_actor_t {
    nk_gc_t gc;
    uint32_t refc;
    uint8_t active;
    nk_frame_t frame;
    nk_msgq_t mailbox;
};

nk_actor_t* nk_actor_spawn();

void nk_actor_free(nk_actor_t* actor);

nk_actor_t* nk_actor_this(nk_actor_t* set);

nk_value nk_actor_recv(nk_actor_t* actor);

void nk_actor_send(nk_actor_t* actor, nk_value value);

#endif // _NK_ACTOR_H