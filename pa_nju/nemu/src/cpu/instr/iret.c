#include "cpu/instr.h"
/*
Put the implementations of `iret' instructions here.
*/

make_instr_func(iret){
    cpu.eip = pop();
    uint16_t cs = pop(); cpu.cs.val = cs;
    load_sreg(SREG_CS);
    cpu.eflags.val = pop();

    return 0;
}
