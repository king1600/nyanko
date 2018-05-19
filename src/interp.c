#include "actor.h"

#ifndef NK_REDUCTIONS
#define NK_REDUCTIONS 2000
#endif

#define Next(ip) goto *((const void**) ((ip)++))

uintptr_t nk_actor_interp(nk_actor_t* actor, uint8_t bytecode) {
    static void* nk_opcode_labels[] = {
        &&op_nop,
        &&op_exit,
        &&op_bif,
        &&op_ldc,
        &&op_lda,
        &&op_ldu,
        &&op_ld,
        &&op_st,
        &&op_sta,
        &&op_stu,
        &&op_cmp,
        &&op_pop_eq,
        &&op_pop_lt,
        &&op_jmp,
        &&op_je,
        &&op_jne,
        &&op_jl,
        &&op_jle,
        &&op_jg,
        &&op_jge,
        &&op_anew,
        &&op_mnew,
        &&op_get,
        &&op_drop,
        &&op_put,
        &&op_load,
        &&op_send,
        &&op_recv,
        &&op_yield,
        &&op_try,
        &&op_catch,
        &&op_call,
        &&op_ret,
        &&op_clz,
        &&op_add,
        &&op_sub,
        &&op_mul,
        &&op_div,
        &&op_mod,
        &&op_shr,
        &&op_shl,
        &&op_xor,
        &&op_and,
        &&op_or,
        &&op_not,
        &&op_is_null,
        &&op_is_int,
        &&op_is_float,
        &&op_is_func,
        &&op_is_str,
        &&op_is_map,
        &&op_is_data,
        &&op_is_array,
        &&op_is_tuple,
        &&op_is_port
    };

    if (actor == NULL)
        return (uintptr_t) nk_opcode_labels[bytecode];

////////////////////////////////////////////////////////
    int reductions = NK_REDUCTIONS;
    nk_frame_t* frame = &actor->frame;

    op_nop:
        Next(frame->ip);

    op_bif:
    op_ldc:
    op_lda:
    op_ldu:
    op_ld:
    op_st:
    op_sta:
    op_stu:
    op_cmp:
    op_pop_eq:
    op_pop_lt:
    op_jmp:
    op_je:
    op_jne:
    op_jl:
    op_jle:
    op_jg:
    op_jge:
    op_anew:
    op_mnew:
    op_get:
    op_drop:
    op_put:
    op_load:
    op_send:
    op_recv:
    op_yield:
    op_try:
    op_catch:
    op_call:
    op_ret:
    op_clz:
    op_add:
    op_sub:
    op_mul:
    op_div:
    op_mod:
    op_shr:
    op_shl:
    op_xor:
    op_and:
    op_or:
    op_not:
    op_is_null:
    op_is_int:
    op_is_float:
    op_is_func:
    op_is_str:
    op_is_map:
    op_is_data:
    op_is_array:
    op_is_tuple:
    op_is_port:

    op_exit:
        nk_actor_free(actor);
        return false;
}