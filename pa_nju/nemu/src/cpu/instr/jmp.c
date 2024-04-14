#include "cpu/instr.h"

make_instr_func(jmp_near){
    OPERAND rel;
    rel.type = OPR_IMM;
    rel.sreg = SREG_CS;
    rel.data_size = data_size;
    rel.addr = eip + 1;

    operand_read(&rel);

    int offset = sign_ext(rel.val, data_size);
    // thank Ting Xu from CS'17 for finding this bug
    print_asm_1("jmp", "", 1 + data_size / 8, &rel);

    cpu.eip += offset;

    return 1 + data_size / 8;
}

make_instr_func(jmp_b){
    OPERAND rel;
    rel.type = OPR_IMM;
    rel.sreg = SREG_CS;
    rel.data_size = 8;
    rel.addr = eip + 1;

    operand_read(&rel);

    int offset = sign_ext(rel.val, 8);
    // thank Ting Xu from CS'17 for finding this bug
    print_asm_1("jmp", "", 2, &rel);

    cpu.eip += offset;

    return 2;
}

make_instr_func(jmp_rm_v){
    int len = 1;
    OPERAND rel;
    rel.data_size = data_size;
    len += modrm_rm(eip + 1, &rel);

    operand_read(&rel);

    // rel.val = sign_ext(rel.val, data_size);
    len = 0;
    print_asm_1("jmp", "", 1 + data_size / 8, &rel);

    cpu.eip = rel.val;

    return len;
}

make_instr_func(jmp_lng_ptr){
    OPERAND opr1, opr2;
    
    // first 16 or 32 bits
    opr1.type = OPR_IMM;
    opr1.sreg = SREG_CS;
    opr1.data_size = data_size;
    opr1.addr = eip + 1;
    operand_read(&opr1);
    
    // last 16 bits, i.e., selector
    opr2.type = OPR_IMM;
    opr2.sreg = SREG_CS;
    opr2.data_size = 16;
    opr2.addr = eip + 1 + data_size / 8;
    operand_read(&opr2);
    
    cpu.eip = opr1.val;
    
    // set SREG_CS
    cpu.segReg[SREG_CS].val = (uint16_t)opr2.val;
    load_sreg(SREG_CS);
    
    return 0;
}