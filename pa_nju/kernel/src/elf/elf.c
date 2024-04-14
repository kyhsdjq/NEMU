#include "common.h"
#include "memory.h"
#include "string.h"

#include <elf.h>

#ifdef HAS_DEVICE_IDE
#define ELF_OFFSET_IN_DISK 0
#endif

#define STACK_SIZE (1 << 20)

#define vpn_mask 0xfffff000

void ide_read(uint8_t *, uint32_t, uint32_t);
void create_video_mapping();
uint32_t get_ucr3();

uint32_t loader()
{
	Elf32_Ehdr *elf;
	Elf32_Phdr *ph, *eph;

#ifdef HAS_DEVICE_IDE
	uint8_t buf[4096];
	ide_read(buf, ELF_OFFSET_IN_DISK, 4096);
	elf = (void *)buf;
	Log("ELF loading from hard disk.");
#else
	elf = (void *)0x0;
// 	BREAK_POINT;
	Log("ELF loading from ram disk.");
// 	BREAK_POINT;
#endif
//  Log("elf = 0x%x", elf);

	/* Load each program segment */
	ph = (void *)elf + elf->e_phoff; // start of pht
	eph = ph + elf->e_phnum; // end of pht

	for (; ph < eph; ph++)
	{
		if (ph->p_type == PT_LOAD)
		{

			// remove this panic!!!
// 			panic("Please implement the loader");
#ifndef IA32_PAGE
/* TODO: copy the segment from the ELF file to its proper memory area */
            memcpy((void *)ph->p_vaddr, (void *)((uint32_t)elf + ph->p_offset), ph->p_filesz);

/* TODO: zeror the memory area [vaddr + file_sz, vaddr + mem_sz] */
            memset((void *)(ph->p_vaddr + ph->p_filesz), 0, ph->p_memsz - ph->p_filesz);
#else       
//          Log("ph = 0x%x", ph);     
            uint32_t physical_addr = (uint32_t)(mm_malloc(ph->p_vaddr, ph->p_memsz));
//          Log("1");
            physical_addr = (physical_addr & vpn_mask) | (ph->p_vaddr & ~vpn_mask);
//          Log("2");
            uint32_t virtual_addr = (uint32_t)(pa_to_va(physical_addr));
//          Log("3");
//          Log("physical_addr = 0x%x, filesz = 0x%x", physical_addr, ph->p_filesz);
//          Log("elf + offset = 0x%x, offset = 0x%x", (uint32_t)elf + ph->p_offset, ph->p_offset);
#ifdef HAS_DEVICE_IDE
            ide_read((uint8_t *)virtual_addr, ELF_OFFSET_IN_DISK + ph->p_offset, ph->p_filesz);
#else
            memcpy((void *)virtual_addr, (void *)((uint32_t)elf + ph->p_offset), ph->p_filesz);
#endif
//          Log("4");
            memset((void *)(virtual_addr + ph->p_filesz), 0x00, ph->p_memsz - ph->p_filesz);
//          Log("5");
#endif

#ifdef IA32_PAGE
			/* Record the program break for future use */
			extern uint32_t brk;
			uint32_t new_brk = ph->p_vaddr + ph->p_memsz - 1;
			if (brk < new_brk)
			{
				brk = new_brk;
			}
#endif
		}
	}
//  Log("For loop finished.");

	volatile uint32_t entry = elf->e_entry;

#ifdef IA32_PAGE
    mm_malloc(KOFFSET - STACK_SIZE, STACK_SIZE);
// 	Log("physical_addr_stack_begin = 0x%x, physical_addr_stack_end = 0x%x", stack_begin, stack_begin + STACK_SIZE);
// 	Log("virtual_addr_stack_begin = 0x%x, virtual_addr_stack_end = 0x%x", KOFFSET - STACK_SIZE, KOFFSET);
// 	BREAK_POINT;
#ifdef HAS_DEVICE_VGA
	create_video_mapping();
#endif
	write_cr3(get_ucr3());
#endif
//  Log("Load finished.");
	return entry;
}
