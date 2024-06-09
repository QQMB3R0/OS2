#ifndef ATA_H
#define ATA_H

#define DISK_READ_LOCATION 40000
#define SECTOR_SIZE 512
#define STAT_DRQ  (1 << 3) // Set when the drive has PIO data to transfer, or is ready to accept PIO data.
#define STAT_ERR  (1 << 0) // Indicates an error occurred. Send a new command to clear it

#include "../inc/types.h"
#include "../io/Port.h"
#include "../io/GlobalObj.h"
#include "../memory/heap.h"
#include "../memory/memory.h"
class AtaDriver
{
    public:
        void displayInfo();
        AtaDriver();
        uint16* ata_read_sector(uint32 lba, uint32 num_blocks);
        int ata_write_sector(uint32 lba, uint32 num_blocks,const uint16* buffer);
};
#endif
