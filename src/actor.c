#include "alloc.h"
#include "actor.h"

NK_THREAD_LOCAL nk_actor_t* __nk_actor = NULL;

nk_actor_t* nk_actor_this(nk_actor_t* set) {
    if (set != NULL)
        __nk_actor = set;
    return __nk_actor;
}

nk_actor_t* nk_actor_spawn() {
    return NULL;
}

void nk_actor_free(nk_actor_t* actor) {

}

nk_value nk_actor_recv(nk_actor_t* actor) {
    return NK_NULL;
}

void nk_actor_send(nk_actor_t* actor, nk_value value) {

}