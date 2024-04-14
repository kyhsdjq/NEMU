#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `out' instructions here.
*/

make_instr_func(out_b){
    OPERAND al, dx;
    al.data_size = 8;
    dx.data_size = 16;

    al.type = OPR_REG; al.addr = REG_AL;
    dx.type = OPR_REG; dx.addr = REG_DX;

    operand_read(&al);
    operand_read(&dx);

    pio_write((uint16_t)dx.val, 1, (uint32_t)al.val);

    return 1;
}

make_instr_func(out_l){
    OPERAND al, dx;
    al.data_size = 32;
    dx.data_size = 16;

    al.type = OPR_REG; al.addr = REG_AL;
    dx.type = OPR_REG; dx.addr = REG_DX;

    operand_read(&al);
    operand_read(&dx);

    pio_write((uint16_t)dx.val, 4, al.val);

    return 1;
}
