#ifndef _NK_VM_H
#define _NK_VM_H

#include "scheduler.h"

typedef struct nk_mod_t nk_mod_t;

typedef struct {
    nk_schedlist_t schedulers;
    nk_mpmc_queue_t idle_scheds;
} nk_vm_t;

struct nk_mod_t {
    uint8_t* code;
};

nk_vm_t* nk_vm_this();

void nk_vm_free(nk_vm_t* vm);

void nk_vm_init(nk_vm_t* vm, int schedulers, int asio_threads);

#endif // _NK_VM_H