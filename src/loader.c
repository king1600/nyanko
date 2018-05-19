#include "vm.h"
#include "alloc.h"

typedef union {
    uint8_t*  u8;
    uint16_t* u16;
    uint32_t* u32;
    int32_t*  i32;
    double*   f64;
} nk_reader_t;

bool nk_mod_load(nk_vm_t* vm, const char* bytecode, size_t size) {
    uint16_t stack;
    nk_reader_t pos = { .u8 = (uint8_t*) bytecode };

    return true;
}