#ifndef __CACHE_H__
#define __CACHE_H__

#include "nemu.h"

#ifdef CACHE_ENABLED    

#define cache_data_size 64
#define cache_line_size 8
#define cache_set_size 128

#define cache_read_cnt 1
#define memory_read_cnt 10

typedef struct{
    bool v;
    uint32_t tag;
    uint8_t data[cache_data_size]; // 64 bytes
} cache_line_t;

typedef struct{
    struct{
        cache_line_t line[cache_line_size];
    } set[cache_set_size];
} Cache;

typedef struct{
    uint32_t tag, set_index, block_offset;
    cache_line_t *cache_line;
} cache_addr_t;

// init the cache
void init_cache();

// write data to cache
void cache_write(paddr_t paddr, size_t len, uint32_t data);

// read data from cache
uint32_t cache_read(paddr_t paddr, size_t len);

typedef struct{
    uint32_t cnt_ori, cnt_cache;
} cache_cnt_t;

extern cache_cnt_t cache_cnt;

#endif

#endif
