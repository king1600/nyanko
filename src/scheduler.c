#include "vm.h"

#define NK_SCHEDULER_QUIT ((nk_actor_t*) -1)

struct nk_sched_t {
    nk_vm_t* vm;
    nk_thread_t id;
    nk_cond_t waiter;
    nk_mutex_t mutex;
    nk_actor_t* actor;
    bool is_active;
    bool is_running;
};

NK_THREAD_FUNC(nk_sched_run, sched) {
    nk_sched_t* scheduler = (nk_sched_t*) sched;
    nk_vm_t* vm = scheduler->vm;
    NK_MEM_THREAD_INIT();

    while (scheduler->actor != NK_SCHEDULER_QUIT) {

        if (scheduler->actor != NULL) {
            // TODO: run actor
        }

        if ((scheduler->actor = (nk_actor_t*) nk_mpmc_queue_pop(&vm->actors)) != NULL)
            continue;

        nk_atomic_store(&scheduler->is_active, false, NK_ATOMIC_RELAXED);
        nk_mpmc_queue_push(&vm->idle_schedulers, (void*) scheduler);

        if (scheduler->actor == NULL)
            nk_atomic_store(&scheduler->is_running, true, NK_ATOMIC_RELAXED);

        nk_mutex_lock(&scheduler->mutex);
        while (!scheduler->is_active)
            nk_cond_wait(&scheduler->waiter, &scheduler->mutex);
        nk_mutex_unlock(&scheduler->mutex);
    }

    NK_MEM_THREAD_FREE();
    nk_thread_exit();
    return NULL;
}

void nk_sched_submit(nk_actor_t* actor) {
    nk_vm_t* vm = nk_vm_this(NULL);
    nk_sched_t* scheduler = (nk_sched_t*) nk_mpmc_queue_pop(&vm->idle_schedulers);

    if (scheduler == NULL) {
        nk_mpmc_queue_push(&vm->actors, (void*) actor);
    } else {
        scheduler->actor = actor;
        nk_atomic_store(&scheduler->is_active, true, NK_ATOMIC_RELAXED);
        nk_cond_signal(&scheduler->waiter);
    }
}

void nk_sched_init(nk_vm_t* vm, nk_schedlist_t* list, size_t amount) {
    nk_sched_t *scheduler;

    list->size = amount = amount < 1 ? nk_cores() : amount;
    list->workers = (nk_sched_t*) NK_MALLOC(sizeof(nk_sched_t) * list->size);

    while (amount--) {
        scheduler = &((nk_sched_t*) list->workers)[amount];
        scheduler->is_running = false;
        scheduler->is_active = false;
        scheduler->actor = NULL;
        scheduler->vm = vm;

        nk_cond_init(&scheduler->waiter);
        nk_mutex_init(&scheduler->mutex);
        scheduler->id = nk_thread_init(nk_sched_run, scheduler);
        
        while (!nk_atomic_load(&scheduler->is_running, NK_ATOMIC_RELAXED))
            nk_thread_yield();
    }
}

void nk_sched_free(nk_schedlist_t* list) {
    nk_sched_t *scheduler;

    size_t size = list->size;
    while (size--)
        nk_sched_submit(NK_SCHEDULER_QUIT);

    while (list->size--) {
        scheduler = &((nk_sched_t*) list->workers)[list->size];
        nk_thread_join(scheduler->id);
        nk_cond_free(&scheduler->waiter);
        nk_mutex_free(&scheduler->mutex);
    }
    
    NK_FREE((void*) list->workers);
}