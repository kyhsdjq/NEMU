#include "cpu/instr.h"
/*
Put the implementations of `ret' instructions here.
*/
make_instr_func(near_ret)
{
    print_asm_0("near_ret", "", 1);
    cpu.eip = pop();
    return 0;
}

#define instr_execute_1op() do{ \
    operand_read(&opr_src); \
    cpu.eip = pop(); \
    cpu.esp += opr_src.val; \
    len = 0; \
} while(0);

make_instr_impl_1op(near_ret, i, w)