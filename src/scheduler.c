#include "vm.h"
#include "alloc.h"

typedef void* (*nk_sched_func) (void* arg);
#define NK_SCHEDULER_QUIT ((nk_actor_t*) -1)
#define __nk_inline(type) static NK_INLINE type

#ifdef NK_WINDOWS
    #include <windows.h>
    #define nk_thread_t HANDLE
    #define nk_mutex_t CRITICAL_SECTION
    #define nk_cond_t CONDITION_VARIABLE
    #define NK_SCHEDULER_WORKER(name) DWORD WINAPI name(PVOID arg)
#else
    #include <pthread.h>
    #include <sys/sysinfo.h>
    #define nk_thread_t pthread_t
    #define nk_cond_t pthread_cond_t
    #define nk_mutex_t pthread_mutex_t
    #define NK_SCHEDULER_WORKER(name) void* name(void* arg)
#endif

typedef struct {
    nk_vm_t* vm;
    nk_thread_t id;
    nk_cond_t waiter;
    nk_mutex_t mutex;
    nk_actor_t* actor;
    bool is_active;
    bool is_running;
} nk_sched_t;

#ifdef NK_WINDOWS
    // get number of cpu cores
    __nk_inline(int) nk_sched_cores() {
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return (int) sysinfo.dwNumberOfProcessor;
    }

    // condition variables
    __nk_inline(void) nk_cond_free(nk_cond_t* cond) {}
    __nk_inline(void) nk_cond_init(nk_cond_t* cond) {
        InitializeConditionVariable(cond);
    }

    // mutexes
    __nk_inline(void) nk_mutex_free(nk_mutex_t* mutex) {}
    __nk_inline(void) nk_mutex_init(nk_mutex_t* mutex) {
        InitializeCriticalSection(mutex);
    }

    // threads
    __nk_inline(void) nk_thread_init(nk_sched_t* scheduler, nk_sched_func func) {
        DWORD thread_id;
        scheduler->id = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)scheduler, 0, &thread_id);
    }
    __nk_inline(void) nk_thread_join(nk_thread_t thread_id) {
        WaitForSingleObject(thread_id, INFINITE);
        CloseHandle(thread_id);
    }
     __nk_inline(void) nk_thread_exit() {
        ExitThread(NULL);
    }

    // scheduler sleep/wakeup
    __nk_inline(void) nk_sched_wakeup(nk_cond_t* cond) {
        WakeConditionVariable(cond);
    }
    __nk_inline(void) nk_sched_sleep(nk_sched_t* scheduler) {
        EnterCriticalSection(&scheduler->mutex);
        while (!scheduler->is_active)
            SleepConditionVariableCS(&scheduler->waiter, &scheduler->mutex, INFINITE);
        LeaveCriticalSection(&scheduler->mutex);
    }


#else
    // get number of cpu cores
    __nk_inline(int) nk_sched_cores() {
        return get_nprocs();
    }

    // condition variables
    __nk_inline(void) nk_cond_init(nk_cond_t* cond) {
        pthread_cond_init(cond, NULL);
    }
    __nk_inline(void) nk_cond_free(nk_cond_t* cond) {
        pthread_cond_destroy(cond);
    }

    // mutexes
    __nk_inline(void) nk_mutex_init(nk_mutex_t* mutex) {
        pthread_mutex_init(mutex, NULL);
    }
    __nk_inline(void) nk_mutex_free(nk_mutex_t* mutex) {
        pthread_mutex_destroy(mutex);
    }

    // threads
    __nk_inline(void) nk_thread_exit() {
        pthread_exit(NULL);
    }
    __nk_inline(void) nk_thread_join(nk_thread_t thread_id) {
        pthread_join(thread_id, NULL); 
    }
    __nk_inline(void) nk_thread_init(nk_sched_t* scheduler, nk_sched_func func) {
        pthread_create(&scheduler->id, NULL, func, (void*) scheduler);
    }

    // scheduler sleep/wakeup
    __nk_inline(void) nk_sched_wakeup(nk_cond_t* cond) {
        pthread_cond_signal(cond);
    }
    __nk_inline(void) nk_sched_sleep(nk_sched_t* scheduler) {
        pthread_mutex_lock(&scheduler->mutex);
        while (!scheduler->is_active)
            pthread_cond_wait(&scheduler->waiter, &scheduler->mutex);
        pthread_mutex_unlock(&scheduler->mutex);
    }
#endif

NK_SCHEDULER_WORKER(nk_sched_run) {
    nk_sched_t* scheduler = (nk_sched_t*) arg;
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
        nk_sched_sleep(scheduler);
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
        nk_sched_wakeup(&scheduler->waiter);
    }
}

void nk_sched_init(nk_vm_t* vm, nk_schedlist_t* list, size_t amount) {
    nk_sched_t *scheduler;

    list->size = amount = amount < 1 ? nk_sched_cores() : amount;
    list->workers = (void*) NK_MALLOC(sizeof(nk_sched_t) * list->size);

    while (amount--) {
        scheduler = &((nk_sched_t*) list->workers)[amount];
        scheduler->is_running = false;
        scheduler->is_active = false;
        scheduler->actor = NULL;
        scheduler->vm = vm;

        nk_cond_init(&scheduler->waiter);
        nk_mutex_init(&scheduler->mutex);
        nk_thread_init(scheduler, nk_sched_run);
        
        while (!nk_atomic_load(&scheduler->is_running, NK_ATOMIC_RELAXED))
            nk_sched_yield();
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
    
    NK_FREE(list->workers);
}