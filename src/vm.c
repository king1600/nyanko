#include "vm.h"

static nk_vm_t* __nk_vm;

nk_vm_t* nk_vm_this(nk_vm_t* vm) {
    if (vm != NULL)
        __nk_vm = vm;
    return __nk_vm;
}

void nk_vm_init(nk_vm_t* vm, int num_scheds, int num_asio) {
    NK_MEM_INIT();
    NK_MEM_THREAD_INIT();

    nk_mutex_init(&vm->modmap_lock);
    nk_imap_init(&vm->mod_map, false, 8);

    nk_mpmc_queue_init(&vm->actors);
    nk_mpmc_queue_init(&vm->idle_schedulers);
    nk_sched_init(vm, &vm->schedulers, num_scheds);
}

void nk_vm_free(nk_vm_t* vm) {
    nk_mutex_free(&vm->modmap_lock);
    nk_imap_free(&vm->mod_map);

    nk_sched_free(&vm->schedulers);
    nk_mpmc_queue_free(&vm->actors);
    nk_mpmc_queue_free(&vm->idle_schedulers);

    NK_MEM_THREAD_FREE();
    NK_MEM_FREE();
}
