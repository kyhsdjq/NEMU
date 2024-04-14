#include "cpu/instr.h"
#include "device/port_io.h"
/*
Put the implementations of `in' instructions here.
*/

make_instr_func(in_b){
    OPERAND al, dx;
    al.data_size = 8;
    dx.data_size = 16;

    al.type = OPR_REG; al.addr = REG_AL;
    dx.type = OPR_REG; dx.addr = REG_DX;

    operand_read(&dx);

    al.val = pio_read((uint16_t)dx.val, 1);

    operand_write(&al);

    return 1;
}

make_instr_func(in_l){
    OPERAND al, dx;
    al.data_size = 32;
    dx.data_size = 16;

    al.type = OPR_REG; al.addr = REG_AL;
    dx.type = OPR_REG; dx.addr = REG_DX;

    operand_read(&dx);

    al.val = pio_read((uint16_t)dx.val, 4);

    operand_write(&al);

    return 1;
}
