#include "cpu/instr.h"
/*
Put the implementations of `lgdt' instructions here.
*/
make_instr_func(lgdt){
    int len = 1;
    opr_src.data_size = 16; opr_dest.data_size = data_size;
    if(opr_dest.data_size == 16) opr_dest.data_size = 24;
    len += modrm_rm(eip + 1, &opr_src);
    modrm_rm(eip + 1, &opr_dest);
    opr_dest.addr += 2;
    print_asm_2("lgdt", opr_dest.data_size == 8 ? "b" : (opr_dest.data_size == 16 ? "w" : "l"), len, &opr_src, &opr_dest);
    operand_read(&opr_src);
    operand_read(&opr_dest);
    cpu.gdtr.limit = opr_src.val;
    cpu.gdtr.base = opr_dest.val;
    
    return len;
}
