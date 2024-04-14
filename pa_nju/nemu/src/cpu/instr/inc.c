#include "cpu/instr.h"
/*
Put the implementations of `inc' instructions here.
*/
static void instr_execute_1op() 
{
	operand_read(&opr_src);
// 	printf("inc fron %d to ", opr_src.val);
	opr_src.val += 1; // not to change carry flag
// 	printf("%d \n", opr_src.val);
	operand_write(&opr_src);
}

make_instr_impl_1op(inc, r, v)
make_instr_impl_1op(inc, rm, v)