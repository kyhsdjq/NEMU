#include "cpu/intr.h"
#include "cpu/instr.h"
#include "memory/memory.h"

void raise_intr(uint8_t intr_no)
{
#ifdef IA32_INTR
	push(cpu.eflags.val);
	push(cpu.cs.val);
	push(cpu.eip);
	
    GateDesc gatedesc;
    gatedesc.val[0] = laddr_read(cpu.idtr.base + intr_no * 8, 4);
    gatedesc.val[1] = laddr_read(cpu.idtr.base + intr_no * 8 + 4, 4);

    if(gatedesc.type == InterruptGateType)
        cpu.eflags.IF = false;

    cpu.cs.val = gatedesc.selector;
    load_sreg(SREG_CS);
    cpu.eip = gatedesc.offset_31_16 << 16 | gatedesc.offset_15_0;
#endif
}

void raise_sw_intr(uint8_t intr_no)
{
	// return address is the next instruction
	cpu.eip += 2;
	raise_intr(intr_no);
}
