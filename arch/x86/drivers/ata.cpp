#include "ata.h"

bool AtaDriver::wait_bit_set(uint8 status_reg_bit)
{
    uint8 status = Port::inb(IO_BASE + R_STATUS_REG);

    for(uint8 i = 0; !(status & status_reg_bit); i++)
    {
        if(i >= WAIT_BIT_DELAY)
        {
            return true;
        }
        status = Port::inb(IO_BASE + R_STATUS_REG);
    }
    return false;

};

bool AtaDriver::wait_bit_unset(uint8 status_reg_bit)
{
    uint8 status = Port::inb(IO_BASE + R_STATUS_REG);

    for(uint8 i = 0; status & status_reg_bit; i++)
    {
        if(i >= WAIT_BIT_DELAY)
        {
            return true;
        }
        status = Port::inb(IO_BASE + R_STATUS_REG);
    }
    return false;
};

void AtaDriver::cache_flush(const drive d)
{
    Port::outb(IO_BASE + DRIVE_REG, LBA | DRIVE_REG_CONST_BITS | (d.id << 4));
    Port::outb(IO_BASE + W_COMMAND_REG, CACHE_FLUSH);

    if(wait_bit_unset(BSY))
    {
        soft_reset();
    }
};

void AtaDriver::soft_reset()
{
    Port::outb(CONTROL_BASE + W_DEVICE_CONTROL_REG, SRST);
    four_ns_delay();
    Port::outb(CONTROL_BASE + W_DEVICE_CONTROL_REG, NULL);
};

void AtaDriver::four_ns_delay()
{
    Port::inb(IO_BASE + R_STATUS_REG);
    Port::inb(IO_BASE + R_STATUS_REG);
    Port::inb(IO_BASE + R_STATUS_REG);
    Port::inb(IO_BASE + R_STATUS_REG);
};

bool AtaDriver::read_data_port(void *buffer)
{
    if(wait_bit_unset(BSY))
    {
        soft_reset();
        return true;
    }

    uint8 status = Port::inb(IO_BASE + R_STATUS_REG);
    size i = 0;
    while(status & DRQ)
    {
        uint8 error = Port::inb(IO_BASE + R_ERROR_REG);
        if(status & ERR)
        {
            this->error = error;
            return true;
        }
        else if(status & BSY and wait_bit_unset(BSY))
        {
            soft_reset();
            return true;
        }

        // Write data to buffer
        uint16 data = Port::inw(IO_BASE + DATA_REG);
        ((uint16_s *)buffer)[i++] = data;
        four_ns_delay();

        status = Port::inb(IO_BASE + R_STATUS_REG);
    }

    this->error = Port::inb(IO_BASE + R_ERROR_REG);
    return status & ERR;
};

bool AtaDriver::write_data_port(const void *buffer)
{
    if(wait_bit_unset(BSY))
    {
        soft_reset();
        return true;
    }

    uint8 status = Port::inb(IO_BASE + R_STATUS_REG);
    size i = 0;
    while(status & DRQ)
    {
        uint8 error = Port::inb(IO_BASE + R_ERROR_REG);
        if(status & ERR)
        {
            this->error = error;
            return true;
        }

        // Write data to port
        Port::outw(IO_BASE + DATA_REG, ((uint16_s *)buffer)[i++]);
        four_ns_delay();

        status = Port::inb(IO_BASE + R_STATUS_REG);
    }

    this->error = Port::inb(IO_BASE + R_ERROR_REG);
    return status & ERR;
};

void AtaDriver::identify(drive d)
{
    uint8 status;

    Port::outb(IO_BASE + DRIVE_REG, DRIVE_REG_CONST_BITS + (d.id << 4));
    Port::outb(IO_BASE + SEC_COUNT_REG, NULL);
    Port::outb(IO_BASE + LBA_LOW, NULL);
    Port::outb(IO_BASE + LBA_MID, NULL);
    Port::outb(IO_BASE + LBA_HIGH, NULL);
    Port::outb(IO_BASE + W_COMMAND_REG, IDENTIFY_CMD);

    four_ns_delay();
    status = Port::inb(IO_BASE + R_STATUS_REG);
    if(status == NULL)
    {
        if(!d.id) is_master_drive_exist = false;
        if(d.id) is_slave_drive_exist = false;
        return;
    }

    if(wait_bit_unset(BSY))
    {
        soft_reset();
        return;
    }

    if(Port::inb(IO_BASE + LBA_MID) or Port::inb(IO_BASE + LBA_HIGH))
    {
        if(!d.id) is_master_ata = false;
        if(d.id) is_slave_ata = false;
    }

    status = Port::inb(R_STATUS_REG);
    if(status & ERR) return;
    else
    {
        four_ns_delay();
        read_data_port(identify_buf);
    }
};

AtaDriver::AtaDriver()
{
    is_master_drive_exist = true;
    is_slave_drive_exist = true;
    is_master_ata = true;
    is_slave_ata = true;

    cache_flush(MASTER_DRIVE);
    cache_flush(SLAVE_DRIVE);

    identify(MASTER_DRIVE);
    identify(SLAVE_DRIVE);
};

/*if you need to read one sector num_blocks = 1*/
char *AtaDriver::ata_read_sector(const uint32 lba, const uint32 num_blocks, const drive d)
{
    Port::outb(0x1F6, LBA | DRIVE_REG_CONST_BITS | (d.id << 4) | ((lba >> 24) & 0xF));
    Port::outb(0x1F1, NULL);
    Port::outb(0x1F2, num_blocks);
    Port::outb(0x1F3, lba & 0xFF);
    Port::outb(0x1F4, (lba >> 8) & 0xFF);
    Port::outb(0x1F5, (lba >> 16) & 0xFF);
    Port::outb(0x1F7, READ_DATA_CMD);

    char *buffer = (char *)(0x12000000); // SWITCH TO KMALLOC!!!

    if (buffer == NULL) return NULL;

    // wait device
     if(wait_bit_set(DRQ))
     {
        uint8 status = Port::inb(IO_BASE + R_STATUS_REG);
        if(status & ERR)
        {
            error = Port::inb(IO_BASE + R_ERROR_REG);
            return nullptr;
        }
     }

    memset(buffer, 0, SECTOR_SIZE * num_blocks);

    if(read_data_port(buffer))
    {
        memset(buffer, 0, SECTOR_SIZE * num_blocks);
        return nullptr;
    }

    return buffer;
}

/*if you need to write one sector num_blocks = 1*/
bool AtaDriver::ata_write_sector(const uint32 lba, const uint32 num_blocks, const char* buffer, const drive d)
{
    Port::outb(0x1F6, LBA | DRIVE_REG_CONST_BITS | (d.id << 4) | ((lba >> 24) & 0xF));
    Port::outb(0x1F1, 0x00);
    Port::outb(0x1F2, num_blocks);
    Port::outb(0x1F3, lba & 0xFF);
    Port::outb(0x1F4, (lba >> 8) & 0xFF);
    Port::outb(0x1F5, (lba >> 16) & 0xFF);
    Port::outb(0x1F7, 0x30);

    if (buffer == NULL) return NULL;

    // Wait device
    if(wait_bit_set(DRQ))
    {
        uint8 status = Port::inb(IO_BASE + R_STATUS_REG);
        if(status & ERR)
        {
            error = Port::inb(IO_BASE + R_ERROR_REG);
            return true;
        }
    }

    bool result = write_data_port(buffer);

    cache_flush(d);
    return false;
};
