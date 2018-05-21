#ifndef _NK_LOADER_H
#define _NK_LOADER_H

#include "map.h"

typedef union {
    uint8_t*  u8;
    uint16_t* u16;
    uint32_t* u32;
    double*   f64;
    uintptr_t* uptr;
} nk_reader_t;

typedef struct nk_func_t nk_func_t;
typedef struct {
    nk_vm_t* vm;
    nk_string_t name;
    nk_value* consts;
    uint16_t num_consts;
    nk_func_t* funcs;
    uint8_t num_funcs;
} nk_mod_t;

struct nk_func_t {
    uint8_t* code;
    nk_string_t name;
    nk_mod_t* module;
    uint16_t mx_stack;
    uint8_t mx_args;
    uint8_t num_locals;
};

#define NK_MOD_BAD_HEADER   1
#define NK_MOD_BAD_CONST    2
#define NK_MOD_LARGE_INT    3
#define NK_MOD_LARGE_STRING 4
#define NK_MOD_BAD_STACK    5

int nk_mod_load(nk_vm_t* vm, const char* bytecode);

#endif // _NK_LOADER_H