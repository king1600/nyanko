#include "vm.h"
#include "alloc.h"
#include "opcodes.h"
#include <string.h>

#define NK_CONST_8  0
#define NK_CONST_16 1
#define NK_CONST_32 2
#define NK_CONST_64 3
#define NK_HEADER "NYAN"
#define NK_CONST_SIZE(s) ((s) >> 3)
#define NK_CONST_TYPE(t) ((t) & 0b111)
#define NK_ARGS_INC(attr) if (value > func->mx_args) func->mx_args = value
#define NK_STACK_INC() if (++stack > func->mx_stack) func->mx_stack = stack; continue

int nk_mod_load(nk_vm_t* vm, const char* bytecode) {
    uint64_t value;
    nk_func_t* func;
    nk_mod_t* module;
    nk_reader_t pos = { .u8 = (uint8_t*) bytecode };

    // check bytecode header
    if (*pos.u32++ != *((uint32_t*) NK_HEADER))
        return NK_MOD_BAD_HEADER;

    // load modules
    size_t num_modules = *pos.u32++;
    while (num_modules--) {

        // module meta-data
        module = (nk_mod_t*) NK_MALLOC(sizeof(nk_mod_t));
        module->name.size = strchr((const char*) pos.u8, 0) - ((char*) pos.u8);
        module->name.data = (uint8_t*) NK_MALLOC(module->name.size);
        memcpy((void*) module->name.data, (const void*) pos.u8, module->name.size);
        pos.u8 += module->name.size;
        module->vm = vm;

        // load module constants
        uint8_t const_type;
        size_t num_consts = module->num_consts = *(pos.u16++);
        module->consts = (nk_value*) NK_CALLOC(sizeof(nk_value), num_consts);
        for (size_t i = 0; i < num_consts; i++) {
            switch (NK_CONST_TYPE(const_type = *pos.u8++)) {

                // floats are inline
                case NK_CONST_FLOAT:
                    module->consts[i] = *pos.f64++;
                    break;

                // modules link to constant string name
                case NK_CONST_MOD_REF:
                    module->consts[i] = NK_VALUE(NK_CONST_MOD_REF, (uintptr_t) (module->consts + (*pos.u16++)));
                    break;

                // integers are variable width (interpreted as i32 however)
                case NK_CONST_INT:
                    value = (NK_NAN | NK_CONST_INT) << NK_SHIFT;
                    switch (NK_CONST_SIZE(const_type)) {
                        case NK_CONST_8: value |= *pos.u8++; break;
                        case NK_CONST_16: value |= *pos.u16++; break;
                        case NK_CONST_32: value |= *pos.u32++; break;
                        default: return NK_MOD_LARGE_INT;
                    }
                    module->consts[i] = NK_FROM(value);
                    break;

                // strings start with vint width size, then byte content
                case NK_CONST_STRING:
                    switch (NK_CONST_SIZE(const_type)) {
                        case NK_CONST_8: value = *pos.u8++; break;
                        case NK_CONST_16: value = *pos.u16++; break;
                        case NK_CONST_32: value = *pos.u32++; break;
                        default: return NK_MOD_LARGE_STRING;
                    }
                    uint32_t* ptr = (uint32_t*) NK_MALLOC(sizeof(uint32_t) + value);
                    *ptr = (uint32_t) value;
                    memcpy((void*) &ptr[1], (const void*) pos.u8, value);
                    pos.u8 += value;
                    module->consts[i] = NK_VALUE(NK_CONST_STRING, (uintptr_t) ptr);
                    break;

                // invalid constant type
                default:
                    return NK_MOD_BAD_CONST;
            }
        }

        // load functions
        uint8_t num_funcs = module->num_funcs = *pos.u8++;
        for (uint8_t opcode, i = 0; i < num_funcs; i++) {
            
            // load function meta-data
            size_t stack = 0;
            uint32_t* name = NK_PTR(uint32_t*, module->consts[*pos.u16++]);
            func = (nk_func_t*) NK_MALLOC(sizeof(nk_func_t));
            func->module = module;
            func->name.size = *name;
            func->num_locals = *pos.u8++;
            func->mx_args = func->mx_stack = 0;
            func->name.data = (uint8_t*) &name[1];

            // load function bytecode
            func->code = pos.u8;
            while ((opcode = *pos.u8++) != NK_OP_RET) {
                switch (opcode) {

                    // u8 argument
                    case NK_OP_CALL: case NK_OP_TCALL:
                    case NK_OP_ST: case NK_OP_STA: case NK_OP_STU:
                    case NK_OP_BIF: case NK_OP_LDC: case NK_OP_LDA: case NK_OP_LD:
                        value = *pos.u8++; break;

                    // u16 argument
                    case NK_OP_ANEW: case NK_OP_ONEW:
                    case NK_OP_JL: case NK_OP_JLE: case NK_OP_JG: case NK_OP_JGE:
                    case NK_OP_JMP: case NK_OP_JE: case NK_OP_JNE: case NK_OP_TRY:
                        value = *pos.u16++; break;

                    // u32 argument
                    case NK_OP_JL_W: case NK_OP_JLE_W: case NK_OP_JG_W: case NK_OP_JGE_W:
                    case NK_OP_JMP_W: case NK_OP_JE_W: case NK_OP_JNE_W: case NK_OP_TRY_W:
                        value = *pos.u32++; break;

                    // no argument
                    default: value = UINT64_MAX;
                }

                // check stack and args
                switch (opcode) {
                    // instructions without stack modification
                    case NK_OP_NOP: case NK_OP_EXIT: case NK_OP_CMP: case NK_OP_RET:
                    case NK_OP_JMP: case NK_OP_JMP_W: case NK_OP_JE: case NK_OP_JE_W:
                    case NK_OP_JNE: case NK_OP_JNE_W: case NK_OP_JL: case NK_OP_JL_W:
                    case NK_OP_JG: case NK_OP_JG_W: case NK_OP_YIELD: case NK_OP_CATCH:
                        continue;

                    // instructions which increment the stack
                    case NK_OP_LDA: NK_ARGS_INC(); NK_STACK_INC();
                    case NK_OP_BIF: case NK_OP_LDC: case NK_OP_LD: case NK_OP_RECV:
                        NK_STACK_INC();

                    // instructions which decrement the stack
                    case NK_OP_SEND: case NK_OP_THROW:
                    case NK_OP_GET: case NK_OP_PUT: case NK_OP_DROP:
                        value = 2;
                    default:
                        if (value == UINT64_MAX) value = 1;
                        if (stack < value) return NK_MOD_BAD_STACK;
                        stack -= value;
                }
            }
        }
    }

    return 0;
}