#ifndef _NYANKO_ACTOR_H
#define _NYANKO_ACTOR_H

#include "nk.h"
#include "atomic.h"

typedef struct {
    void** heap;
    bool barrier;
    nk_ring_t grays;
    uint64_t* hp_map;
    uint32_t hp_size;
} nk_gc_t;

typedef struct {
    int32_t* ip;
    nk_value* bp;
    nk_value func;
    nk_value reg[0xff];
} nk_frame_t;

struct nk_actor_t {
    nk_gc_t gc;
    uint64_t id;
    nk_frame_t frame;
    nk_queue_t mailbox;
};



#endif // _NYANKO_ACTOR_H