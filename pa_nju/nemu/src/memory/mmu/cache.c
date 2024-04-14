#include "memory/mmu/cache.h"
#include "memory/memory.h"

#include <stdlib.h>

Cache cache;

cache_cnt_t cache_cnt;

#define tag_mask 0xffffe000
#define set_index_mask 0x00001fc0
#define block_offset_mask 0x0000003f
#define paddr_mask 0xffffffc0

#define set_cache_line(cache_addr, data_addr) \
    cache_addr.cache_line->v = true; \
    cache_addr.cache_line->tag = cache_addr.tag; \
    memcpy(cache_addr.cache_line->data, (uint8_t *)(data_addr), cache_data_size);

#define split_paddr(paddr, cache_addr) \
    cache_addr.tag = ((paddr) & tag_mask) >> 13; \
    cache_addr.set_index = ((paddr) & set_index_mask) >> 6; \
    cache_addr.block_offset = (paddr) & block_offset_mask; \
    cache_addr.cache_line = 0;

static void erase_cache(){
    int s, e;
    for(s = 0; s < cache_set_size; s ++){
        for(e = 0; e < cache_line_size; e ++){
            cache.set[s].line[e].v = false;
        }
    }
}

void init_cache_cnt(){
    cache_cnt.cnt_ori = 0;
    cache_cnt.cnt_cache = 0;
}

// init the cache
void init_cache()
{
	// implement me in PA 3-1
	erase_cache();
	init_cache_cnt();
}

bool cache_search(cache_addr_t *cache_addr){
    bool flag_v;
    uint32_t tag, set_index;
    int e;
    int random_factor;
    flag_v = false;
    tag = cache_addr->tag;
    set_index = cache_addr->set_index;
    
    cache_addr->cache_line = &cache.set[set_index].line[0];
    
    for(e = 0; e < cache_line_size; e ++){
        if(cache.set[set_index].line[e].v){
            if(cache.set[set_index].line[e].tag == tag){
                cache_addr->cache_line = &cache.set[set_index].line[e];
                return true;
            }
        }
        else if(!flag_v){
            // store the first empty line
            cache_addr->cache_line = &cache.set[set_index].line[e];
            flag_v = true;
        }
    }
    
    if(!flag_v){
        random_factor = rand() % cache_line_size;
        cache_addr->cache_line = &cache.set[set_index].line[random_factor];
    }
    
    return false;
}

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data)
{
	// implement me in PA 3-1
	cache_addr_t cache_addr1, cache_addr2; 
	
	split_paddr(paddr, cache_addr1)
	split_paddr(paddr + len - 1, cache_addr2)
	if(cache_addr1.set_index == cache_addr2.set_index){
        if(cache_search(&cache_addr1)){
            memcpy(&cache_addr1.cache_line->data[cache_addr1.block_offset], &data, len);
        }
	}
	else{
	    uint32_t len1, len2;
	    
	    len1 = cache_data_size - cache_addr1.block_offset;
	    len2 = cache_addr2.block_offset + 1;
	    
	    // low pos
	    if(cache_search(&cache_addr1))
	        memcpy(&cache_addr1.cache_line->data[cache_addr1.block_offset], &data, len1);
        // high pos
        if(cache_search(&cache_addr2))
            memcpy(&cache_addr2.cache_line->data[0], (void *)&data + len1, len2);
	}
	
    hw_mem_write(paddr, len, data); // write through
}

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len)
{
	// implement me in PA 3-1
	uint32_t ret = 0;
	cache_addr_t cache_addr1, cache_addr2;
	
	split_paddr(paddr, cache_addr1);
	split_paddr(paddr + len - 1, cache_addr2)
	
	if(cache_addr1.set_index == cache_addr2.set_index){
	    if(cache_search(&cache_addr1)){
	        // get data from cache
	        memcpy(&ret, &cache_addr1.cache_line->data[cache_addr1.block_offset], len);
	        cache_cnt.cnt_cache += cache_read_cnt;
	    }
	    else{
	        // get data from memory
	        memcpy(&ret, hw_mem + paddr, len);
	        cache_cnt.cnt_cache += memory_read_cnt;
	        
	        // put data into cache
	        set_cache_line(cache_addr1, hw_mem + (paddr & paddr_mask));
	    }
	    cache_cnt.cnt_ori += memory_read_cnt;
	}
	else{
	    uint32_t len1, len2;
	    uint32_t ret1, ret2;
	    
	    len1 = cache_data_size - cache_addr1.block_offset;
	    len2 = cache_addr2.block_offset + 1;
	    
	    // low pos
	    if(cache_search(&cache_addr1)){
	        // get data from cache
	        memcpy(&ret1, &cache_addr1.cache_line->data[cache_addr1.block_offset], len1);
	        cache_cnt.cnt_cache += cache_read_cnt;
	    }
	    else{
	        // get data from memory
	        memcpy(&ret1, hw_mem + paddr, len1);
	        cache_cnt.cnt_cache += memory_read_cnt;
	        
	        // put data into cache
	        set_cache_line(cache_addr1, hw_mem + (paddr & paddr_mask));
	    }
	    
	    // high pos
	    if(cache_search(&cache_addr2)){
	        // get data from cache
	        memcpy(&ret2, &cache_addr2.cache_line->data[0], len2);
	        cache_cnt.cnt_cache += cache_read_cnt;
	    }
	    else{
	        // get data from memory
	        memcpy(&ret2, hw_mem + paddr + len1, len2);
	        cache_cnt.cnt_cache += memory_read_cnt;
	        
	        // put data into cache
	        set_cache_line(cache_addr2, hw_mem + ((paddr + len - 1) & paddr_mask));
	    }
	    
	    memcpy(&ret, &ret1, len1);
	    memcpy((void *)&ret + len1, &ret2, len2);
	    cache_cnt.cnt_ori += 2 * memory_read_cnt; // read twice seperately
	}
	
	return ret;
}

