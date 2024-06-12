#ifndef ATA_H
#define ATA_H

#include "../io/Port.h"
#include "../io/GlobalObj.h"
#include "../memory/heap.h"
#include "../memory/memory.h"

#define DISK_READ_LOCATION 40000
#define SECTOR_SIZE 512
#define STAT_DRQ  (1 << 3) // Set when the drive has PIO data to transfer, or is ready to accept PIO data.
#define STAT_ERR  (1 << 0) // Indicates an error occurred. Send a new command to clear it

// ATA ports defines
#define IO_BASE 0x1f0
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
#define DRIVE_REG_CONST_BITS 0xa0

// Command register defines
#define READ_DATA_CMD 0x20
#define IDENTIFY_CMD 0xec
#define CACHE_FLUSH 0xe7

// Status register defines
#define BSY 0x80
#define DRQ 0x08
#define ERR 0x01

#define IDENTIFY_BUF_SZ 256

#define WAIT_BIT_DELAY 50

#define MASTER_DRIVE 0
#define SLAVE_DRIVE 1

// Error register defines
#define AMNF 1
#define TKZNF (1 << 1)
#define ABRT (1 << 2)
#define MCR (1 << 3)
#define IDNF (1 << 4)
#define MC (1 << 5)
#define UNC (1 << 6)
#define BBK (1 << 7)

using size = uint32;

struct drive
{
    uint16 id : 1;
    drive(uint16 v) : id(v){};
};

class AtaDriver
{
    private:
        uint8 error;
        uint16_s identify_buf[IDENTIFY_BUF_SZ];
        bool is_master_drive_exist, is_slave_drive_exist;
        bool is_master_ata, is_slave_ata;
    public:
        AtaDriver();
        /**
          * @retrun NULL if error. Error read property recomend.
          *  Otherwise return pointer to duffer.
          */
        char * ata_read_sector(
                               const uint32 lba,
                               const uint32 num_blocks,
                               const drive d = MASTER_DRIVE
                               );
        /**
          * @return true if error. Recommend. Error read property recomend.
          * False if data was write in buffer.
          */
        bool ata_write_sector(
                             const uint32 lba,
                             const uint32 num_blocks,
                             const char* buffer,
                             const drive d = MASTER_DRIVE
                             );
        void four_ns_delay();
        /**
          * @return error property.
          */
        uint8 getError();
        void soft_reset();
        void identify(drive d);
        /**
          * @eturn true if ERR bit in the status register set
          */
        bool read_data_port(void *buffer);
        /**
          * @return true if ERR bit in the status register set
          */
        bool write_data_port(const void *buffer);
        /**
          * @return true if bit is not set. False in otherwise.
          */
        bool wait_bit_set(uint8 status_reg_bit);
        /**
          * @return true if bit is not unset. False in otherwise.
          */
        bool wait_bit_unset(uint8 status_reg_bit);

        /**
          * Flush disk cache
          *
          * @param d Drive id (master or slave)
          */
        void cache_flush(const drive d);
};
#endif
