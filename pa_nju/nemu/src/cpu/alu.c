#include "cpu/cpu.h"

void set_PF(uint32_t result, size_t data_size){
    result ^= result >> 4;
    result ^= result >> 2;
    result ^= result >> 1;
    cpu.eflags.PF = !(result & 0x1);
    return;
}

void set_ZF(uint32_t result, size_t data_size){
    cpu.eflags.ZF = !(result & (0xFFFFFFFF >> (32 - data_size)));
    return;
}

void set_SF(uint32_t result, size_t data_size){
    cpu.eflags.SF = !!(result & (0x1 << (data_size - 1)));
    return;
}




void set_add_CF(uint32_t result, uint32_t src, size_t data_size) {
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	cpu.eflags.CF = result < src;
	return;
}

void set_add_OF(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    if(src >> 31 == dest >> 31){
        cpu.eflags.OF = (src >> 31) ^ (result >> 31);
    }
    else{
        cpu.eflags.OF = 0;
    }
    return;
}

uint32_t alu_add(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_add(src, dest, data_size);
#else
    uint32_t result = src + dest;
    
    set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    set_add_CF(result, src, data_size);
    set_add_OF(result, src, dest, data_size);
    
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}





void set_adc_CF(uint32_t result, uint32_t src, size_t data_size) {
	result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
	src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
	if(cpu.eflags.CF){
	    cpu.eflags.CF = result <= src;
	}
	else{
	    cpu.eflags.CF = result < src;
	}
	return;
}

void set_adc_OF(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    if(src >> 31 == dest >> 31){
        cpu.eflags.OF = (src >> 31) ^ (result >> 31);
    }
    else{
        cpu.eflags.OF = 0;
    }
    return;
}

uint32_t alu_adc(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_adc(src, dest, data_size);
#else
	uint32_t result = src + dest + cpu.eflags.CF;
    
    set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    set_adc_CF(result, src, data_size);
    set_adc_OF(result, src, dest, data_size);
    
	return result & (0xFFFFFFFF >> (32 - data_size));
	
	return 0;
#endif
}




void set_sub_CF(uint32_t result, uint32_t dest, size_t data_size){
    result = sign_ext((result & (0xFFFFFFFF >> (32 - data_size))), data_size);
    dest = sign_ext((dest & (0xFFFFFFFF >> (32 - data_size))), data_size);
    cpu.eflags.CF = result > dest;
    return;
}

void set_sub_OF(uint32_t result, uint32_t src, uint32_t dest, size_t data_size){
    result = sign_ext(result & (0xFFFFFFFF >> (32 - data_size)), data_size);
    src = sign_ext(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    dest = sign_ext(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
    
    if(src >> 31 != dest >> 31){
        cpu.eflags.OF = (dest >> 31) ^ (result >> 31);
    }
    else{
        cpu.eflags.OF = 0;
    }
    
    return;
}

uint32_t alu_sub(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sub(src, dest, data_size);
#else
	uint32_t result = dest - src;
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    set_sub_CF(result, dest, data_size);
    set_sub_OF(result, src, dest, data_size);
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}




void set_sbb_CF(uint32_t result, uint32_t dest, size_t data_size){
    result = sign_ext((result & (0xFFFFFFFF >> (32 - data_size))), data_size);
    dest = sign_ext((dest & (0xFFFFFFFF >> (32 - data_size))), data_size);
    if(cpu.eflags.CF){
        cpu.eflags.CF = result >= dest;
    }
    else{
        cpu.eflags.CF = result > dest;
    }
    
    return;
}

uint32_t alu_sbb(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sbb(src, dest, data_size);
#else
	uint32_t result = dest - src - cpu.eflags.CF;
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    set_sbb_CF(result, dest, data_size);
    set_sub_OF(result, src, dest, data_size);
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}





void set_mul_CF(uint64_t result, size_t data_size){
    cpu.eflags.CF = !!(result >> data_size);
    
    return;
}

void set_mul_OF(uint64_t result, size_t data_size){
    cpu.eflags.OF = !!(result >> data_size);
    
    return;
}

uint64_t alu_mul(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mul(src, dest, data_size);
#else
    uint64_t temp_s = (uint64_t)(src & (0xFFFFFFFF >> (32 - data_size)));
    uint64_t temp_d = (uint64_t)(dest & (0xFFFFFFFF >> (32 - data_size)));
	uint64_t result = temp_d * temp_s;

	
    set_mul_CF(result, data_size);
    set_mul_OF(result, data_size);
	
	return result & ((uint64_t)-1 >> (64 - 2 * data_size));
#endif
}



void set_imul_CF(int64_t result, size_t data_size){
    if(result & (0x1 << (data_size - 1))){
        cpu.eflags.CF = !!~(result >> data_size);
    }
    else{
        cpu.eflags.CF = !!(result >> data_size);
    }
    
    
    return;
}

void set_imul_OF(int64_t result, size_t data_size){
    if(result & (0x1 << (data_size - 1))){
        cpu.eflags.OF = !!~(result >> data_size);
    }
    else{
        cpu.eflags.OF = !!(result >> data_size);
    }
    
    return;
}

int64_t alu_imul(int32_t src, int32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imul(src, dest, data_size);
#else
	int64_t temp_s = sign_ext_64(src & (0xFFFFFFFF >> (32 - data_size)), data_size);
    int64_t temp_d = sign_ext_64(dest & (0xFFFFFFFF >> (32 - data_size)), data_size);
	int64_t result = temp_d * temp_s;

	
    set_imul_CF(result, data_size);
    set_imul_OF(result, data_size);
	
	return result;
	return 0;
#endif
}

// need to implement alu_mod before testing
uint32_t alu_div(uint64_t src, uint64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_div(src, dest, data_size);
#else
	uint64_t result = dest / src;
	
	
	return (uint32_t)(result & (0xFFFFFFFF >> data_size));
#endif
}

// need to implement alu_imod before testing
int32_t alu_idiv(int64_t src, int64_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_idiv(src, dest, data_size);
#else
	int64_t result = dest / src;
	
	
	return (int32_t)(result & (0xFFFFFFFF >> data_size));
#endif
}

uint32_t alu_mod(uint64_t src, uint64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_mod(src, dest);
#else
	uint64_t result = dest % src;
	
	
	return (uint32_t)result;
#endif
}

int32_t alu_imod(int64_t src, int64_t dest)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_imod(src, dest);
#else
	int64_t result = dest % src;
	
	
	return (int32_t)result;
#endif
}






uint32_t alu_and(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_and(src, dest, data_size);
#else
	uint32_t result = src & dest;
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_xor(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_xor(src, dest, data_size);
#else
	uint32_t result = src ^ dest;
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_or(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_or(src, dest, data_size);
#else
	uint32_t result = src | dest;
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    cpu.eflags.CF = 0;
    cpu.eflags.OF = 0;
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}




void set_shl_CF(uint32_t src, uint32_t dest, size_t data_size){
    cpu.eflags.CF = (dest >> (data_size - src)) & 0x1;
    
    return;
}

void set_shl_OF(uint32_t src, uint32_t dest, size_t data_size){
    if(src == 1){
        cpu.eflags.OF = (dest >> (data_size - 2)) & 0x1;
    }
    
    return;
}

uint32_t alu_shl(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shl(src, dest, data_size);
#else
	uint32_t result = dest << src;
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    set_shl_CF(src, dest, data_size);
    set_shl_OF(src, dest, data_size);
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}



void set_shr_CF(uint32_t src, uint32_t dest, size_t data_size){
    cpu.eflags.CF = (dest >> (src - 1)) & 0x1;
    
    return;
}

void set_shr_OF(uint32_t src, uint32_t result, size_t data_size){
    if(src == 1){
        cpu.eflags.OF = result >> (data_size - 1);
    }
    
    return;
}

uint32_t alu_shr(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_shr(src, dest, data_size);
#else
    dest &= 0xFFFFFFFF >> (32 - data_size);
	uint32_t result = dest >> src;
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    set_shr_CF(src, dest, data_size);
    set_shr_OF(src, result, data_size);
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}




void set_sar_CF(uint32_t src, uint32_t dest, size_t data_size){
    cpu.eflags.CF = (dest >> (src - 1)) & 0x1;
    
    return;
}

void set_sar_OF(uint32_t src, uint32_t result, size_t data_size){
    cpu.eflags.OF = 0;
    
    return;
}


uint32_t alu_sar(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sar(src, dest, data_size);
#else
    int32_t temp = sign_ext(dest, data_size);
    
	uint32_t result = (uint32_t)(temp >> src);
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    set_sar_CF(src, dest, data_size);
    set_sar_OF(src, result, data_size);
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}

uint32_t alu_sal(uint32_t src, uint32_t dest, size_t data_size)
{
#ifdef NEMU_REF_ALU
	return __ref_alu_sal(src, dest, data_size);
#else
	uint32_t result = dest << src;
	
	set_PF(result, data_size);
    set_ZF(result, data_size);
    set_SF(result, data_size);
    set_shl_CF(src, dest, data_size);
    set_shl_OF(src, dest, data_size);
	
	return result & (0xFFFFFFFF >> (32 - data_size));
#endif
}
