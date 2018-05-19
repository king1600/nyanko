#ifndef _NK_OPCODE
#define _NK_OPCODE

#define NK_OP_NOP    0
#define NK_OP_NNK_OP_LABEL nk_op_NOP

#define NK_OP_EXIT   1
#define NK_OP_EXIT_LABEL nk_op_EXIT

#define NK_OP_BIF    2
#define NK_OP_BIF_LABEL nk_op_BIF

#define NK_OP_LDC    3
#define NK_OP_LDC_LABEL nk_op_LDC

#define NK_OP_LDA    4
#define NK_OP_LDA_LABEL nk_op_LDA

#define NK_OP_LDU    5
#define NK_OP_LDU_LABEL nk_op_LDU

#define NK_OP_LD     6
#define NK_OP_LD_LABEL nk_op_LD

#define NK_OP_ST     7
#define NK_OP_ST_LABEL nk_op_ST

#define NK_OP_STA    8
#define NK_OP_STA_LABEL nk_op_STA

#define NK_OP_STU    9
#define NK_OP_STU_LABEL nk_op_STU

#define NK_OP_CMP    10
#define NK_OP_CMP_LABEL nk_op_CMP

#define NK_OP_JMP    11
#define NK_OP_JMP_LABEL nk_op_JMP

#define NK_OP_JMP_W  12
#define NK_OP_JMP_W_LABEL nk_op_JMP_W

#define NK_OP_JE     13
#define NK_OP_JE_LABEL nk_op_JE

#define NK_OP_JE_W   14
#define NK_OP_JE_W_LABEL nk_op_JE_W

#define NK_OP_JNE    15
#define NK_OP_JNE_LABEL nk_op_JNE

#define NK_OP_JNE_W  16
#define NK_OP_JNE_W_LABEL nk_op_JNE_W

#define NK_OP_JL     17
#define NK_OP_JL_LABEL nk_op_JL

#define NK_OP_JL_W   18
#define NK_OP_JL_W_LABEL nk_op_JL_W

#define NK_OP_JLE    19
#define NK_OP_JLE_LABEL nk_op_JLE

#define NK_OP_JLE_W  20
#define NK_OP_JLE_W_LABEL nk_op_JLE_W

#define NK_OP_JG     21
#define NK_OP_JG_LABEL nk_op_JG

#define NK_OP_JG_W   22
#define NK_OP_JG_W_LABEL nk_op_JG_W

#define NK_OP_JGE    23
#define NK_OP_JGE_LABEL nk_op_JGE

#define NK_OP_JGE_W  24
#define NK_OP_JGE_W_LABEL nk_op_JGE_W

#define NK_OP_ANEW   25
#define NK_OP_ANEW_LABEL nk_op_ANEW

#define NK_OP_ONEW   26
#define NK_OP_ONEW_LABEL nk_op_ONEW

#define NK_OP_GET    27
#define NK_OP_GET_LABEL nk_op_GET

#define NK_OP_DROP   28
#define NK_OP_DROP_LABEL nk_op_DROP

#define NK_OP_PUT    29
#define NK_OP_PUT_LABEL nk_op_PUT

#define NK_OP_MLOAD  30
#define NK_OP_MLOAD_LABEL nk_op_MLOAD

#define NK_OP_SEND   31
#define NK_OP_SEND_LABEL nk_op_SEND

#define NK_OP_RECV   32
#define NK_OP_RECV_LABEL nk_op_RECV

#define NK_OP_YIELD  33
#define NK_OP_YIELD_LABEL nk_op_YIELD

#define NK_OP_TRY    34
#define NK_OP_TRY_LABEL nk_op_TRY

#define NK_OP_TRY_W  35
#define NK_OP_TRY_W_LABEL nk_op_TRY_W

#define NK_OP_THROW  36
#define NK_OP_THROW_LABEL nk_op_THROW

#define NK_OP_CATCH  37
#define NK_OP_CATCH_LABEL nk_op_CATCH

#define NK_OP_CALL   38
#define NK_OP_CALL_LABEL nk_op_CALL

#define NK_OP_TCALL  39
#define NK_OP_TCALL_LABEL nk_op_TCALL

#define NK_OP_RET    40
#define NK_OP_RET_LABEL nk_op_RET

#define NK_OP_CLZ    41
#define NK_OP_CLZ_LABEL nk_op_CLZ

#define NK_OP_INULL  42
#define NK_OP_INULL_LABEL nk_op_INULL

#define NK_OP_IINT   43
#define NK_OP_IINT_LABEL nk_op_IINT

#define NK_OP_IFLOAT 44
#define NK_OP_IFLOAT_LABEL nk_op_IFLOAT

#define NK_OP_IFUNC  45
#define NK_OP_IFUNC_LABEL nk_op_IFUNC

#define NK_OP_ISTR   46
#define NK_OP_ISTR_LABEL nk_op_ISTR

#define NK_OP_IMAP   47
#define NK_OP_IMAP_LABEL nk_op_IMAP

#define NK_OP_IDATA  48
#define NK_OP_IDATA_LABEL nk_op_IDATA

#define NK_OP_IARRAY 49
#define NK_OP_IARRAY_LABEL nk_op_IARRAY

#define NK_OP_ITUPLE 50
#define NK_OP_ITUPLE_LABEL nk_op_ITUPLE

#define NK_OP_IPORT  51
#define NK_OP_IPORT_LABEL nk_op_IPORT

#define NK_OP_ADD    52
#define NK_OP_ADD_LABEL nk_op_ADD

#define NK_OP_SUB    53
#define NK_OP_SUB_LABEL nk_op_SUB

#define NK_OP_MUL    54
#define NK_OP_MUL_LABEL nk_op_MUL

#define NK_OP_DIV    55
#define NK_OP_DIV_LABEL nk_op_DIV

#define NK_OP_MOD    56
#define NK_OP_MOD_LABEL nk_op_MOD

#define NK_OP_SHR    57
#define NK_OP_SHR_LABEL nk_op_SHR

#define NK_OP_SHL    58
#define NK_OP_SHL_LABEL nk_op_SHL

#define NK_OP_XOR    59
#define NK_OP_XOR_LABEL nk_op_XOR

#define NK_OP_AND    60
#define NK_OP_AND_LABEL nk_op_AND

#define NK_OP_OR     61
#define NK_OP_OR_LABEL nk_op_OR

#define NK_OPCODE_LABELS \
    &&NK_OP_NOP_LABEL, \
    &&NK_OP_EXIT_LABEL, \
    &&NK_OP_BIF_LABEL, \
    &&NK_OP_LDC_LABEL, \
    &&NK_OP_LDA_LABEL, \
    &&NK_OP_LD_LABEL, \
    &&NK_OP_ST_LABEL, \
    &&NK_OP_STA_LABEL, \
    &&NK_OP_STU_LABEL, \
    &&NK_OP_CMP_LABEL, \
    &&NK_OP_JMP_LABEL, \
    &&NK_OP_JMP_W_LABEL, \
    &&NK_OP_JE_LABEL, \
    &&NK_OP_JE_W_LABEL, \
    &&NK_OP_JNE_LABEL, \
    &&NK_OP_JNE_W_LABEL, \
    &&NK_OP_JL_LABEL, \
    &&NK_OP_JL_W_LABEL, \
    &&NK_OP_JLE_LABEL, \
    &&NK_OP_JLE_W_LABEL, \
    &&NK_OP_JG_LABEL, \
    &&NK_OP_JG_W_LABEL, \
    &&NK_OP_JGE_LABEL, \
    &&NK_OP_JGE_W_LABEL, \
    &&NK_OP_ANEW_LABEL, \
    &&NK_OP_ONEW_LABEL, \
    &&NK_OP_GET_LABEL, \
    &&NK_OP_DROP_LABEL, \
    &&NK_OP_PUT_LABEL, \
    &&NK_OP_MLOAD_LABEL, \
    &&NK_OP_SEND_LABEL, \
    &&NK_OP_RECV_LABEL, \
    &&NK_OP_YIELD_LABEL, \
    &&NK_OP_TRY_LABEL, \
    &&NK_OP_TRY_W_LABEL, \
    &&NK_OP_THROW_LABEL, \
    &&NK_OP_CATCH_LABEL, \
    &&NK_OP_CALL_LABEL, \
    &&NK_OP_TCALL_LABEL, \
    &&NK_OP_RET_LABEL, \
    &&NK_OP_CLZ_LABEL, \
    &&NK_OP_INULL_LABEL, \
    &&NK_OP_IINT_LABEL, \
    &&NK_OP_IFLOAT_LABEL, \
    &&NK_OP_IFUNC_LABEL, \
    &&NK_OP_ISTR_LABEL, \
    &&NK_OP_IMAP_LABEL, \
    &&NK_OP_IDATA_LABEL, \
    &&NK_OP_IARRAY_LABEL, \
    &&NK_OP_ITUPLE_LABEL, \
    &&NK_OP_IPORT_LABEL, \
    &&NK_OP_ADD_LABEL, \
    &&NK_OP_SUB_LABEL, \
    &&NK_OP_MUL_LABEL, \
    &&NK_OP_DIV_LABEL, \
    &&NK_OP_MOD_LABEL, \
    &&NK_OP_SHR_LABEL, \
    &&NK_OP_SHL_LABEL, \
    &&NK_OP_XOR_LABEL, \
    &&NK_OP_AND_LABEL, \
    &&NK_OP_OR_LABEL 

#endif // _NK_OPCODE