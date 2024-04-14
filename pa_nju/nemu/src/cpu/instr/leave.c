#include "cpu/instr.h"
/*
Put the implementations of `leave' instructions here.
*/
make_instr_func(leave)
{
    print_asm_0("leave", "", 1);
    cpu.esp = cpu.ebp;
    cpu.ebp = pop();
    return 1;
}