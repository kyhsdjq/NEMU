#include "cpu/intr.h"
#include "cpu/instr.h"

/*
Put the implementations of `int' instructions here.

Special note for `int': please use the instruction name `int_' instead of `int'.
*/

#define instr_execute_1op() \
do{ \
    operand_read(&opr_src); \
    uint8_t intr_no = opr_src.val; \
    raise_sw_intr(intr_no); \
    len = 0; \
}while(0);

make_instr_impl_1op(int, i, b);
