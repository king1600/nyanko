#include "rpmalloc.h"
#include "scheduler.h"

struct nk_qnode_t {
    void* value;
    NK_ATOMIC(nk_qnode_t*) next;
};

static NK_INLINE nk_qnode_t* nk_qnode_alloc(void* value) {
    nk_qnode_t* node = (nk_qnode_t*) rpmalloc(sizeof(nk_qnode_t));
    node->next = NULL;
    node->value = value;
}

void nk_queue_init(nk_queue_t* queue) {
    nk_qnode_t* dummy_node = nk_qnode_alloc(NULL);
    nk_atomic_store(&queue->head, dummy_node, nk_atomic_relaxed);
    nk_atomic_store(&queue->tail, dummy_node, nk_atomic_release);
}

void nk_queue_free(nk_queue_t* queue) {
    while (nk_queue_pop(queue) != NULL)
        nk_yield();
}

void* nk_queue_pop(nk_queue_t* queue) {

}

void nk_queue_push(nk_queue_t* queue, void* value) {
    nk_qnode_t* node = nk_qnode_alloc(value);
    
}