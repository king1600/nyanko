#include "actor.h"
#include "alloc.h"
#include "opcodes.h"

#ifndef NK_REDUCTIONS
#define NK_REDUCTIONS 2000
#endif

#define Next(ip) goto *((const void**) ((ip)++))
#define NK_LABEL(name) NK_OP_ ## name ## _LABEL
#define NK_OP(name, body) NK_LABEL(name): body; Next(frame->ip)

uintptr_t nk_actor_interp(nk_actor_t* actor, uint8_t bytecode) {
    static void* nk_opcode_labels[] = { NK_OPCODE_LABELS };
    if (actor == NULL)
        return (uintptr_t) nk_opcode_labels[bytecode];

    int reductions = NK_REDUCTIONS;
    nk_frame_t* frame = &actor->frame;

    NK_OP(NOP, Next(frame->ip));

    NK_OP(BIF, );
    NK_OP(LDC, );
    NK_OP(LDA, );
    NK_OP(LD, );
    NK_OP(ST, );
    NK_OP(STA, );
    NK_OP(STU, );

    NK_OP(CMP, );
    NK_OP(INULL, );
    NK_OP(IINT, );
    NK_OP(IFLOAT, );
    NK_OP(IFUNC, );
    NK_OP(ISTR, );
    NK_OP(IMAP, );
    NK_OP(IDATA, );
    NK_OP(IARRAY, );
    NK_OP(ITUPLE, );
    NK_OP(IPORT, );

    NK_LABEL(JMP_W):
    NK_OP(JMP, );
    NK_LABEL(JE_W):
    NK_OP(JE, );
    NK_LABEL(JNE_W):
    NK_OP(JNE, );
    NK_LABEL(JL_W):
    NK_OP(JL, );
    NK_LABEL(JLE_W):
    NK_OP(JLE, );
    NK_LABEL(JG_W):
    NK_OP(JG, );
    NK_LABEL(JGE_W):
    NK_OP(JGE, );

    NK_OP(ANEW, );
    NK_OP(ONEW, );
    NK_OP(GET, );
    NK_OP(DROP, );
    NK_OP(PUT, );

    NK_OP(MLOAD, );
    NK_OP(SEND, );
    NK_OP(RECV, );
    NK_OP(YIELD, );

    NK_LABEL(TRY_W):
    NK_OP(TRY, );
    NK_OP(THROW, );
    NK_OP(CATCH, );
    NK_OP(CALL, );
    NK_OP(TCALL, );
    NK_OP(RET, );
    NK_OP(CLZ, );

    NK_OP(ADD, );
    NK_OP(SUB, );
    NK_OP(MUL, );
    NK_OP(DIV, );
    NK_OP(MOD, );
    NK_OP(SHR, );
    NK_OP(SHL, );
    NK_OP(XOR, );
    NK_OP(AND, );
    NK_OP(OR, );

    NK_OP(EXIT,
        nk_actor_free(actor);
        return 0;
    );

    NK_UNREACHABLE();
}