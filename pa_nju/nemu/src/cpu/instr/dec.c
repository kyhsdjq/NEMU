#include "cpu/instr.h"
/*
Put the implementations of `dec' instructions here.
*/
static void instr_execute_1op() 
{
	operand_read(&opr_src);
	opr_src.val -= 1; // not to change carry flag
	operand_write(&opr_src);
}

make_instr_impl_1op(dec, r, v)
make_instr_impl_1op(dec, rm, v)