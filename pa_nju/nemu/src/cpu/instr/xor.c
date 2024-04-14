#include "cpu/instr.h"
/*
Put the implementations of `xor' instructions here.
*/
static void instr_execute_2op() 
{
	operand_read(&opr_src);
	operand_read(&opr_dest);
	opr_src.val = sign_ext(opr_src.val, opr_src.data_size); 
// 	opr_src.val = (uint32_t)((int32_t)(opr_src.val << (32 - opr_src.data_size))) >> (32 - opr_src.data_size);
	opr_dest.val = alu_xor(opr_src.val, opr_dest.val, opr_dest.data_size);
	operand_write(&opr_dest);
}

make_instr_impl_2op_all(xor)