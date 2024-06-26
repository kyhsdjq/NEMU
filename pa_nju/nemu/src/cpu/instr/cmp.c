#include "cpu/instr.h"
/*
Put the implementations of `cmp' instructions here.
*/

static void instr_execute_2op() 
{
	operand_read(&opr_src);
	operand_read(&opr_dest);
	opr_src.val = sign_ext(opr_src.val, opr_src.data_size); 
    // opr_dest.val = opr_dest.val & (0xffffffff >> (32 - opr_dest.data_size)); // bv->b
    // printf("compare %08x and %08x with size %d \n", opr_src.val, opr_dest.val, opr_dest.data_size);
	alu_sub(opr_src.val, opr_dest.val, opr_dest.data_size);
}

make_instr_impl_2op_all(cmp)
