#include "actor.h"
#include "rpmalloc.h"

__thread nk_actor_t* __this_actor = NULL;

nk_actor_t* nk_actor_this() {
    return __this_actor;
}

nk_value nk_this() {
    return nk_actor_this()->frame.vthis;
}

nk_value nk_actor_id(nk_actor_t* actor) {
    return nk_alloc_int(actor->id);
}

bool nk_actor_send(nk_actor_t* actor, nk_value value) {
    return false;
}

void nk_actor_free(nk_actor_t* actor) {
    nk_gc_collect(actor, true);
    rpfree((void*) actor->gc.heap_map);
    rpfree((void*) actor->frame.stack);
}

nk_actor_t* nk_actor_spawn(nk_value func, nk_value* args, int n_args) {
    nk_actor_t* actor = (nk_actor_t*) rpmalloc(sizeof(nk_actor_t));
    nk_queue_init(&actor->mailbox);

    // TODO: app nk_func_t structure
    size_t frame_size = (sizeof(nk_value) * n_args); // + (sizeof(nk_value * fn->n_locals);
    actor->frame.stack = (nk_value*) rpmalloc(frame_size);
    actor->frame.bp = actor->frame.stack + n_args;

    actor->gc.heap_size = 1;
    actor->gc.heap_map = (uint64_t*)(actor->gc.heap = NULL);
    
}