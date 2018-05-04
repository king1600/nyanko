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
   return node;
}

void nk_queue_init(nk_queue_t* queue) {
    nk_qnode_t* dummy_node = nk_qnode_alloc(NULL);
    queue->head = dummy_node;
    queue->tail = dummy_node;
}

void nk_queue_free(nk_queue_t* queue) {
    while (nk_queue_pop(queue) != NULL)
        nk_yield();
}

void nk_queue_push(nk_queue_t* queue, void* value) {
    NK_ATOMIC(nk_qnode_t*) tail;
    NK_ATOMIC(nk_qnode_t*) next;
    const nk_qnode_t* null_node = NULL;
    nk_qnode_t* node = nk_qnode_alloc(value);

    while (true) {
        tail = nk_atomic_load(&queue->tail, nk_atomic_acquire);
        if ((next = nk_atomic_load(&tail->next, nk_atomic_acquire)) != NULL)
            nk_atomic_cas_strong(&queue->tail, &tail, next);
        else if (nk_atomic_cas_weak(&tail->next, &null_node, node, nk_atomic_relaxed, nk_atomic_relaxed))
            return;
    }

    NK_UNREACHABLE();
}

void* nk_queue_pop(nk_queue_t* queue) {
    NK_ATOMIC(nk_qnode_t*) head;
    NK_ATOMIC(nk_qnode_t*) tail;
    NK_ATOMIC(nk_qnode_t*) next;

    while (true) {
        head = nk_atomic_load(&queue->head, nk_atomic_acquire);
        tail = nk_atomic_load(&queue->tail, nk_atomic_acquire);
        next = nk_atomic_load(&head->next,  nk_atomic_acquire);

        if (head == tail) {
            if (next == NULL)
                return NULL;
            nk_atomic_cas_strong(&queue->tail, &tail, next);
        } else {
            void* value = nk_atomic_load(&next->value, nk_atomic_relaxed);
            if (nk_atomic_cas_weak(&queue->head, &head, next, nk_atomic_relaxed, nk_atomic_relaxed))
                return value;
        }
    }

    NK_UNREACHABLE();
}

void nk_ring_free(nk_ring_t* ring) {
    rpfree(ring->data);
}

void nk_ring_init(nk_ring_t* ring, size_t size) {
    ring->size = NK_POW2(size);
    ring->rear = ring->front = -1;
    ring->data = (void**) rpmalloc(sizeof(void*) * ring->size);
}

bool nk_ring_push(nk_ring_t* ring, void* value) {
    const size_t size = ring->size - 1;
    if ((ring->front == 0 && ring->rear == size) || (ring->front == ring->rear + 1))
        return true;

    if (ring->rear == size && ring->front == 0)
        ring->rear = -1;
    ring->data[++ring->rear] = value;
    if (ring->front == -1)
        ring->front = 0;

    return false;
}

void* nk_ring_pop(nk_ring_t* ring) {
    if (ring->front == -1 && ring->rear == -1)
        return NULL;

    void* value = ring->data[ring->front++];
    if (ring->front == ring->size)
        ring->front = 0;
    if (ring->front - 1 == ring->rear)
        ring->front = ring->rear = -1;
        
    return value;
}