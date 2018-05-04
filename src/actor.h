#ifndef _NYANKO_ACTOR_H
#define _NYANKO_ACTOR_H

#include "gc.h"

typedef struct {
    int32_t* ip;
    nk_value* bp;
    nk_value func;
    nk_value vthis;
    nk_value* stack;
    nk_value reg[64];
    size_t stack_size;
} nk_frame_t;

struct nk_actor_t {
    nk_gc_t gc;
    uint32_t id;
    nk_frame_t frame;
    nk_queue_t mailbox;
};

#endif // _NYANKO_ACTOR_H