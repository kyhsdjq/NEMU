#include "cpu/instr.h"
/*
Put the implementations of `lidt' instructions here.
*/
make_instr_func(lidt){
    int len = 1;
    opr_src.data_size = 16; opr_dest.data_size = data_size;
    if(opr_dest.data_size == 16) opr_dest.data_size = 24;
    len += modrm_rm(eip + 1, &opr_src);
    modrm_rm(eip + 1, &opr_dest);
    opr_dest.addr += 2;
    print_asm_2("lidt", opr_dest.data_size == 8 ? "b" : (opr_dest.data_size == 16 ? "w" : "l"), len, &opr_src, &opr_dest);
    operand_read(&opr_src);
    operand_read(&opr_dest);
    cpu.idtr.limit = opr_src.val;
    cpu.idtr.base = opr_dest.val;
    
    return len;
}
