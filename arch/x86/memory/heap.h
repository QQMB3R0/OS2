#define HEAP_H
#ifdef HEAP_H

#include "../inc/types.h"
#include "memory.h"
#include "phys_mem.h"
#include "virt_mem.h"

#define PAGE_SIZE               4096
#define MAX_PAGE_ALIGNED_ALLOCS 32

typedef struct memBlock
{
    uint32 size;
    bool free;
    uint32 v_addr;
    uint32 pcount;
    struct memBlock* next;
} mem_block;
extern mem_block *kmalloc_list_head;
extern mem_block *umalloc_list_head;

extern uint32 malloc_virt_address;
extern uint32 kmalloc_phys_addresss;
extern uint32 total_malloc_pages;

void merge_free_blocks(mem_block* block);
void split_block(mem_block* node, const uint32 size);
void kmalloc_init(const uint32 bytes);
void* kmallocp(uint32 v_addr) ;
void* kmalloc(const uint32 size) ;
void* malloc(uint32 size) ;
void free(void* ptr) ;
void* realloc(void* ptr, uint32 size) ;
void kfree(void* ptr);

#endif
