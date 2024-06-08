#define PHYS_MEM_H
#ifdef PHYS_MEM_H
#include "../inc/types.h"
#define BLOCK_SIZE      4096     // Size of 1 block of memory, 4KB
#define BLOCKS_PER_BYTE 8        // Using a bitmap, each byte will hold 8 bits/blocks
// Global variables
extern uint32* memory_map;
extern uint32 max_blocks;
extern uint32 used_blocks;
void set_block(const uint32 bit);
int32 find_first_free_blocks(const uint32 num_blocks);
uint32 *allocate_blocks(const uint32 num_blocks);
#endif