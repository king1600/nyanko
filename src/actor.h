#ifndef _NK_ACTOR_H
#define _NK_ACTOR_H

#include "gc.h"
#include "loader.h"
#include "atomic.h"

typedef struct nk_pos_t {
    nk_func_t* func;
    uint64_t resume;
    struct nk_pos_t* prev;
} nk_pos_t;

typedef struct {
    nk_value* bp;
    nk_value* sp;
    nk_pos_t* tp;
    nk_pos_t* rp;
    nk_reader_t ip;
    nk_value* stack;
    size_t stack_size;
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

void nk_actor_free(nk_actor_t* actor);

bool nk_actor_interp(nk_actor_t* actor);

nk_actor_t* nk_actor_this(nk_actor_t* set);

nk_value nk_actor_recv(nk_actor_t* actor);

void nk_actor_send(nk_actor_t* actor, nk_value value);

nk_actor_t* nk_actor_spawn(nk_func_t* func, uint8_t argc, nk_value* argv);

#endif // _NK_ACTOR_H