#include "cpu/cpu.h"
#include "memory/memory.h"

// return the linear address from the virtual address and segment selector
uint32_t segment_translate(uint32_t offset, uint8_t sreg)
{
	/* TODO: perform segment translation from virtual address to linear address
	 * by reading the invisible part of the segment register 'sreg'
	 */
	return offset + cpu.segReg[sreg].base;
}

// load the invisible part of a segment register
void load_sreg(uint8_t sreg)
{
	/* TODO: load the invisibile part of the segment register 'sreg' by reading the GDT.
	 * The visible part of 'sreg' should be assigned by mov or ljmp already.
	 */
	assert(cpu.segReg[sreg].ti == 0); // GDT
	
    SegDesc descriptor;
    // printf("0x%x \n", cpu.gdtr.base + 8 * cpu.segReg[sreg].index); fflush(stdout);
    descriptor.val[0] = paddr_read(cpu.gdtr.base + 8 * cpu.segReg[sreg].index, 4);
    descriptor.val[1] = paddr_read(cpu.gdtr.base + 8 * cpu.segReg[sreg].index + 4, 4);
    
    cpu.segReg[sreg].base = (descriptor.base_23_16 << 16) | descriptor.base_15_0;
    cpu.segReg[sreg].limit = (descriptor.limit_19_16 << 16) | descriptor.limit_15_0;
    // cpu.segReg[sreg].type = ;
    // cpu.segReg[sreg].privilege_level = ;
    
    // assert for flat module
    assert(cpu.segReg[sreg].base == 0);
    assert(cpu.segReg[sreg].limit == 0xfffff);
    assert(descriptor.granularity == 1);
}
