#include "cpu/instr.h"
/*
Put the implementations of `call' instructions here.
*/
#define instr_execute_1op() \
    do{ \
    operand_read(&opr_src); \
    uint32_t temp_val = sign_ext(opr_src.val, opr_src.data_size); \
    push(cpu.eip + len); \
    cpu.eip += temp_val;} \
    while(0)

make_instr_impl_1op(call, i, v)

#undef instr_execute_1op
#define instr_execute_1op() \
    do{ \
        operand_read(&opr_src); \
        push(cpu.eip + len); \
        cpu.eip = opr_src.val; \
        len = 0; \
    } \
    while(0)



make_instr_impl_1op(call, rm, v)