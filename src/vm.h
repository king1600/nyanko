#ifndef _NK_VM_H
#define _NK_VM_H

#include "thread.h"
#include "buffer.h"
#include "scheduler.h"

struct nk_vm_t {
    nk_imap_t mod_map;
    nk_mutex_t modmap_lock;
    nk_mpmc_queue_t actors;
    nk_schedlist_t schedulers;
    nk_mpmc_queue_t idle_schedulers;
};

nk_vm_t* nk_vm_this();

void nk_vm_free(nk_vm_t* vm);

void nk_vm_init(nk_vm_t* vm, int num_schedulers, int num_asio_threads);

#endif // _NK_VM_H