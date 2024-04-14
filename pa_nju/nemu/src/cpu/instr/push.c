#include "cpu/instr.h"
/*
Put the implementations of `push' instructions here.
*/
static void instr_execute_1op() {
    operand_read(&opr_src);
    opr_src.val = sign_ext(opr_src.val, opr_src.data_size);
    push(opr_src.val);
}

make_instr_impl_1op(push, r, v)
make_instr_impl_1op(push, rm, v)
make_instr_impl_1op(push, i, b)
make_instr_impl_1op(push, i, v)

make_instr_func(pusha){
    uint32_t cpu_esp = cpu.esp;

    push(cpu.eax);
    push(cpu.ecx);
    push(cpu.edx);
    push(cpu.ebx);
    push(cpu_esp);
    push(cpu.ebp);
    push(cpu.esi);
    push(cpu.edi);

    return 1;
}
