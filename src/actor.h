#ifndef _NK_ACTOR_H
#define _NK_ACTOR_H

#include "gc.h"
#include "loader.h"
#include "atomic.h"

typedef struct {
    nk_func_t* func;
    uint64_t* resume;
} nk_trap_t;

typedef struct {
    uint64_t* ip;
    nk_value* bp;
    nk_value* stack;
    nk_value* stack_end;
    nk_trap_t* trap;
    nk_trap_t* traps_end;
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

uintptr_t nk_actor_interp(nk_actor_t* actor, uint8_t bytecode);

#endif // _NK_ACTOR_H