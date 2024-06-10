#ifndef ATA_H
#define ATA_H

#include "../inc/types.h"
#include "../io/Port.h"
#include "../io/GlobalObj.h"
#include "../memory/heap.h"
#include "../memory/memory.h"

#define DISK_READ_LOCATION 40000
#define SECTOR_SIZE 512
#define STAT_DRQ  (1 << 3) // Set when the drive has PIO data to transfer, or is ready to accept PIO data.
#define STAT_ERR  (1 << 0) // Indicates an error occurred. Send a new command to clear it

// ATA ports defines
#define IO_BASE_MASTER 0x1f0
#define IO_BASE_SLAVE 0x170
// Offset for IO port base
#define DATA_REG 0
#define R_ERROR_REG 1
#define W_FEATURE_REG 1
#define SEC_COUNT_REG 2
#define LBA_LOW 3
#define LBA_MID 4
#define LBA_HIGH 5
#define DRIVE_REG 6
#define R_STATUS_REG 7
#define W_COMMAND_REG 7
//---------------------
#define CONTROL_BASE 0x1f7
// Offset for control port base
#define R_ASTATUS_REG 0
#define W_DEVICE_CONTROL_REG 0
// Device control register bits define
#define SRST (1 << 2)
#define R_DRIVE_ADDR_REG 1

// Drive / Head Registers defines
#define DRV_SLAVE (1 << 4)
#define LBA (1 << 6)
#define DRIVE_REG_CONST_BITS 0xA0

// Command register defines
#define READ_DATA 0x20

#define IDENTIFY_BUF_SZ 256

// Error register defines
#define AMNF 1
#define TKZNF (1 << 1)
#define ABRT (1 << 2)
#define MCR (1 << 3)
#define IDNF (1 << 4)
#define MC (1 << 5)
#define UNC (1 << 6)
#define BBK (1 << 7)

struct port
{
    uint16 value : 12;
    port(uint16 v) : value(v){};
};


class AtaDriver
{
    private:
        uint8 error;
        uint16 identifal_buf[IDENTIFY_BUF_SZ];
        bool is_master_drive_exist, is_slave_drive_exist;
    public:
        AtaDriver();
        uint16* ata_read_sector(uint32 lba, uint32 num_blocks);
        int ata_write_sector(uint32 lba, uint32 num_blocks,const uint16* buffer);
        void four_ns_delay(struct port base);
        uint8 getError();
        void soft_reset();
        void identify(struct port base);
};
#endif
