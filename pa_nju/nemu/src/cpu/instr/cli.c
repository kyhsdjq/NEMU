#include "cpu/instr.h"
/*
Put the implementations of `cli' instructions here.
*/

make_instr_func(cli){
    cpu.eflags.IF = false;
    return 1;
}
