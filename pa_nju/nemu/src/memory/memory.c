#include "nemu.h"
#include "cpu/cpu.h"
#include "memory/memory.h"
#include "device/mm_io.h"
#include <memory.h>
#include <stdio.h>

#define vpn_mask 0xfffff000

uint8_t hw_mem[MEM_SIZE_B];

uint32_t hw_mem_read(paddr_t paddr, size_t len)
{
	uint32_t ret = 0;
	memcpy(&ret, hw_mem + paddr, len);
	return ret;
}

void hw_mem_write(paddr_t paddr, size_t len, uint32_t data)
{
	memcpy(hw_mem + paddr, &data, len);
}

uint32_t paddr_read(paddr_t paddr, size_t len) 
{
	uint32_t ret = 0;
#ifdef HAS_DEVICE_VGA
    int map_NO = is_mmio(paddr);
    if(map_NO != -1)
        return mmio_read(paddr, len, map_NO);
#endif

#ifdef CACHE_ENABLED
		ret = cache_read(paddr, len); // read through cache
// 		ret = hw_mem_read(paddr, len);
#else
		ret = hw_mem_read(paddr, len);
#endif
	return ret;
}

void paddr_write(paddr_t paddr, size_t len, uint32_t data) 
{
#ifdef HAS_DEVICE_VGA
    int map_NO = is_mmio(paddr);
    if(map_NO != -1)
        mmio_write(paddr, len, data, map_NO);
#endif

#ifdef CACHE_ENABLED
		cache_write(paddr, len, data); // write through cache
        // hw_mem_write(paddr, len, data);
#else
		hw_mem_write(paddr, len, data);
#endif
}

uint32_t laddr_read(laddr_t laddr, size_t len)
{
#ifdef IA32_PAGE
    if(cpu.cr0.pg){
        // printf("laddr_read \n");
        // fflush(stdout);
        laddr_t laddr_end = laddr + len - 1;
        if((laddr & vpn_mask) == (laddr_end & vpn_mask)){
            // printf("hi \n"); fflush(stdout);
            paddr_t paddr = page_translate(laddr);
            // printf("ih \n"); fflush(stdout);
            return paddr_read(paddr, len);
        }
        else{
            // printf("hi \n"); fflush(stdout);
            // printf("len = 0x%x\n", len); fflush(stdout);
            // printf("laddr = 0x%x, laddr_end = 0x%x \n", laddr, laddr_end); fflush(stdout);
            paddr_t paddr_0 = page_translate(laddr);
            paddr_t paddr_1 = page_translate(laddr_end & vpn_mask);
            // printf("paddr_0 = 0x%x, paddr_1 = 0x%x \n", paddr_0, paddr_1); fflush(stdout);
            uint32_t ret, ret_0, ret_1;
            uint32_t len_0 = 0x1000 - (laddr & ~vpn_mask);
            uint32_t len_1 = (laddr_end & ~vpn_mask) + 1;
            // printf("len_0 = 0x%x, len_1 = 0x%x \n", len_0, len_1); fflush(stdout);
            ret_0 = paddr_read(paddr_0, len_0);
            ret_1 = paddr_read(paddr_1, len_1);
            memcpy(&ret, &ret_0, len_0);
            memcpy((void *)&ret + len_0, &ret_1, len_1);
            // printf("ih \n"); fflush(stdout);
            return ret;
        }
    }
    else{
        // if(cpu.eip > 0xc0000000){
        //     printf("cr0.pg = %d \n", cpu.cr0.pg);
        //     printf("laddr_read %x \n", laddr);
        //     fflush(stdout);
        // }
        uint32_t result = paddr_read(laddr, len);
        // if(cpu.eip > 0xc0000000){
        //     printf("read successfully \n");
        //     fflush(stdout);
        // }
        return result;
    }
#else
	return paddr_read(laddr, len);
#endif
}

void laddr_write(laddr_t laddr, size_t len, uint32_t data)
{
#ifdef IA32_PAGE
    if(cpu.cr0.pg){
        // printf("laddr_write \n");
        // fflush(stdout);
        laddr_t laddr_end = laddr + len - 1;
        if((laddr & vpn_mask) == (laddr_end & vpn_mask)){
            paddr_t paddr = page_translate(laddr);
            paddr_write(paddr, len, data);
        }
        else{
            paddr_t paddr_0 = page_translate(laddr);
            paddr_t paddr_1 = page_translate(laddr_end & vpn_mask);
            uint32_t len_0 = 0x1000 - (laddr & ~vpn_mask);
            uint32_t len_1 = (laddr_end & ~vpn_mask) + 1;
            uint32_t data_1 = 0x00000000;
            memcpy(&data_1, (void *)(&data) + len_0, len_1);
            paddr_write(paddr_0, len_0, data);
            paddr_write(paddr_1, len_1, data_1);
        }
    }
    else
        paddr_write(laddr, len, data);
#else
	paddr_write(laddr, len, data);
#endif
}

uint32_t vaddr_read(vaddr_t vaddr, uint8_t sreg, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_SEG
	vaddr = segment_translate(vaddr, sreg);
#endif
	return laddr_read(vaddr, len);
}

void vaddr_write(vaddr_t vaddr, uint8_t sreg, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
#ifdef IA32_SEG
	vaddr = segment_translate(vaddr, sreg);
#endif
	laddr_write(vaddr, len, data);
}

void init_mem()
{
	// clear the memory on initiation
	memset(hw_mem, 0, MEM_SIZE_B);
#ifdef CACHE_ENABLED
	init_cache();                             // 初始化cache
#endif
#ifdef TLB_ENABLED
	make_all_tlb();
	init_all_tlb();
#endif
}

uint32_t instr_fetch(vaddr_t vaddr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	return vaddr_read(vaddr, SREG_CS, len);
}

uint8_t *get_mem_addr()
{
	return hw_mem;
}

void push(uint32_t val) {
    vaddr_write(cpu.esp - 4, SREG_SS, 4, val);
    cpu.esp -= 4;
    // printf("pushed %2x \n", val);
}

uint32_t pop() {
    cpu.esp += 4;
    return vaddr_read(cpu.esp - 4, SREG_SS, 4);
    // printf("poped %2x \n", ret);
}
