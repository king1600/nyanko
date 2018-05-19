#ifndef _NK_LOADER_H
#define _NK_LOADER_H

#include "map.h"

typedef struct {
    nk_string_t name;
    uintptr_t* consts; // tagged pointers
    uint16_t const_size;
    uint64_t* code;
} nk_mod_t;

typedef struct {
    uint64_t* pos;
    nk_string_t name;
    nk_mod_t* module;
} nk_func_t;

bool nk_mod_load(nk_vm_t* vm, const char* bytecode, size_t size);

#endif // _NK_LOADER_H