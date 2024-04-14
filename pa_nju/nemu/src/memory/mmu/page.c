#include "cpu/cpu.h"
#include "memory/memory.h"

// copied from tlb.c
typedef union {
	struct
	{
		uint32_t pf_off : 12;
		uint32_t pt_idx : 10;
		uint32_t pdir_idx : 10;
	};
	uint32_t addr;
} PageAddr;

// copied from tlb.c
paddr_t page_walk(laddr_t laddr)
{
	PageAddr *addr = (void *)&laddr;
	paddr_t pdir_base = cpu.cr3.val & ~PAGE_MASK;

	PDE pde;
	pde.val = paddr_read(pdir_base + addr->pdir_idx * 4, 4);
	if(!pde.present) {
		printf("eip = 0x%x, laddr = 0x%x, pdir_base = 0x%x, &pde = 0x%x, pde = 0x%x \n", cpu.eip, laddr, pdir_base, pdir_base + addr->pdir_idx * 4, pde.val); fflush(stdout);
		assert(0);
	}
// 	printf("eip = 0x%x, laddr = 0x%x, pdir_base = 0x%x, &pde = 0x%x, pde = 0x%x \n", cpu.eip, laddr, pdir_base, pdir_base + addr->pdir_idx * 4, pde.val); fflush(stdout);
	assert(pde.present);

	paddr_t pt_base = pde.val & ~PAGE_MASK;
	PTE pte;
	
// 	printf("hi \n"); fflush(stdout);
// 	printf("pt_base = %x \npt_inx = %x \n", pt_base, addr->pt_idx); fflush(stdout);
	pte.val = paddr_read(pt_base + addr->pt_idx * 4, 4);
// 	printf("ih \n"); fflush(stdout);
	if(!pte.present) {
        printf("eip = 0x%x, laddr = 0x%x, pdir_base = 0x%x, &pde = 0x%x, pde = 0x%x \n", cpu.eip, laddr, pdir_base, pdir_base + addr->pdir_idx * 4, pde.val); fflush(stdout);
		printf("eip = 0x%x, laddr = 0x%x, pt_base = 0x%x, &pte = 0x%x, pte = 0x%x \n", cpu.eip, laddr, pt_base, pt_base + addr->pdir_idx * 4, pte.val); fflush(stdout);
		assert(0);
	}
	assert(pte.present);
// 	printf("laddr = 0x%x \n", laddr); fflush(stdout);
// 	printf("pte.val = 0x%x \n", pte.val); fflush(stdout);
//  printf("pdir_base = 0x%x \n", pdir_base); fflush(stdout);
//  printf("pt_base = 0x%x, laddr = 0x%x \n", pt_base, laddr); fflush(stdout);
//  assert(0);

	return pte.val;
}

// translate from linear address to physical address
paddr_t page_translate(laddr_t laddr)
{
#ifndef TLB_ENABLED
	return (page_walk(laddr) & ~PAGE_MASK) | (laddr & PAGE_MASK);
#else
	return tlb_read(laddr) | (laddr & PAGE_MASK);
#endif
}
