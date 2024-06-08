#include "phys_mem.h"
 uint32 *memory_map = 0;
 uint32 max_blocks  = 0;
 uint32 used_blocks = 0;
// Sets a block/bit in the memory map (mark block as used)
void set_block(const uint32 bit)
{
    // Divide bit by 32 to get 32bit "chunk" of memory containing bit to be set;
    // Shift 1 by remainder of bit divided by 32 to get bit to set within the
    //   32 bit chunk
    // OR to set that bit
    memory_map[bit/32] |= (1 << (bit % 32));
}
// Find the first free blocks of memory for a given size
int32 find_first_free_blocks(const uint32 num_blocks)
{
    if (num_blocks == 0) return -1; // Can't return no memory, error

    // Test 32 blocks at a time
    for (uint32 i = 0; i < max_blocks / 32;  i++) {
        if (memory_map[i] != 0xFFFFFFFF) {
            // At least 1 bit is not set within this 32bit chunk of memory,
            //   find that bit by testing each bit
            for (int32 j = 0; j < 32; j++) {
                int32 bit = 1 << j;

                // If bit is unset/0, found start of a free region of memory
                if (!(memory_map[i] & bit)) {
                    for (uint32 count = 0, free_blocks = 0; count < num_blocks; count++) {
                        if ((j+count > 31) && (i+1 <= max_blocks / 32)) {
                            if (!(memory_map[i+1] & (1 << ((j+count)-32))))
                                free_blocks++;
                        } else {
                            if (!(memory_map[i] & (1 << (j+count))))
                                free_blocks++;
                        }

                        if (free_blocks == num_blocks) // Found enough free space
                            return i*32 + j;
                    }
                }
            }
        }
    }

    return -1;  // No free region of memory large enough
}
// Allocate blocks of memory
uint32 *allocate_blocks(const uint32 num_blocks)
{
    // If # of free blocks left is not enough, we can't allocate any more, return
    if ((max_blocks - used_blocks) <= num_blocks) return 0;   

    int32 starting_block = find_first_free_blocks(num_blocks);
    if (starting_block == -1) return 0;     // Couldn't find that many blocks in a row to allocate

    // Found free blocks, set them as used
    for (uint32 i = 0; i < num_blocks; i++)
        set_block(starting_block + i);

    used_blocks += num_blocks;  // Blocks are now used/reserved, increase count

    // Convert blocks to bytes to get start of actual RAM that is now allocated
    uint32 address = starting_block * BLOCK_SIZE; 

    return (uint32 *)address;  // Physical memory location of allocated blocks
}

