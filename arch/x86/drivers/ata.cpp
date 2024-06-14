#include "ata.h"

bool AtaDriver::is_drive_exist(const drive d)
{
    if(d.id == MASTER_DRIVE) return is_master_drive_exist;
    else return is_slave_drive_exist;
}

void write_data_port::perform(void *buffer, size indx)
{
    Port::outw(IO_BASE + DATA_REG, ((uint16_s *)buffer)[indx]);
};

void read_data_port::perform(void *buffer, const size indx)
{
    uint16 data = Port::inw(IO_BASE + DATA_REG);
    ((uint16_s *)buffer)[indx] = data;
};

uint8 AtaDriver::get_error()
{
    return error;
};

bool AtaDriver::wait_bit_set(uint8 status_reg_bit)
{
    uint8 status = Port::inb(IO_BASE + R_STATUS_REG);

    for(uint16 i = 0; !(status & status_reg_bit); i++)
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

    for(uint16 i = 0; status & status_reg_bit; i++)
    {
        if(i >= WAIT_BIT_DELAY)
        {
            if(status_reg_bit == BSY) soft_reset();
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

    wait_bit_unset(BSY);
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

bool AtaDriver::data_port_do_operation(data_port_op *operation, void *buffer)
{
    uint8 status;

    if(wait_bit_unset(BSY) or wait_bit_set(DRQ))
    {
        status = Port::inb(IO_BASE + R_STATUS_REG);
        if(status & ERR)
        {
            error = Port::inb(IO_BASE + R_ERROR_REG);
        }
        return true;
    }

    status = Port::inb(IO_BASE + R_STATUS_REG);
    size i = 0;

    do
    {
        while(status & DRQ)
        {
            if(status & ERR)
            {
                uint8 error = Port::inb(IO_BASE + R_ERROR_REG);
                this->error = error;
                return true;
            }

            operation->perform(buffer, i++);
            four_ns_delay();

            status = Port::inb(IO_BASE + R_STATUS_REG);
        }

        if(wait_bit_unset(BSY))
        {
            return true;
        }

        status = Port::inb(IO_BASE + R_STATUS_REG);
    } while(status & DRQ);

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

    if(wait_bit_unset(BSY)) return;

    if(Port::inb(IO_BASE + LBA_MID) and Port::inb(IO_BASE + LBA_HIGH))
    {
        if(!d.id) is_master_ata = false;
        if(d.id) is_slave_ata = false;
        return;
    }

    status = Port::inb(R_STATUS_REG);
    if(status & ERR) return;
    else
    {
        four_ns_delay();
        read_data_port read_port;
        data_port_do_operation(&read_port, identify_buf);
    }
};

AtaDriver::AtaDriver()
{
    is_master_drive_exist = true;
    is_slave_drive_exist = true;
    is_master_ata = true;
    is_slave_ata = true;

    cache_flush(MASTER_DRIVE);
    identify(MASTER_DRIVE);

    cache_flush(SLAVE_DRIVE);
    identify(SLAVE_DRIVE);
};

/*if you need to read one sector num_blocks = 1*/
char *AtaDriver::ata_read_sector(const uint32 lba, const uint32 num_blocks, const drive d)
{
    if(!is_drive_exist(d)) return nullptr;

    // Preapare ports and pass command to device
    Port::outb(0x1F6, LBA | DRIVE_REG_CONST_BITS | (d.id << 4) | ((lba >> 24) & 0xF));
    Port::outb(0x1F1, NULL);
    Port::outb(0x1F2, num_blocks);
    Port::outb(0x1F3, lba & 0xFF);
    Port::outb(0x1F4, (lba >> 8) & 0xFF);
    Port::outb(0x1F5, (lba >> 16) & 0xFF);
    Port::outb(0x1F7, READ_DATA_CMD);

    char *buffer = (char *)(0x12000000);// SWITCH TO KMALLOC!!!
    if (buffer == NULL) return NULL;

    memset(buffer, 0, SECTOR_SIZE * num_blocks);

    read_data_port read_port;
    if(data_port_do_operation(&read_port, buffer))
    {
        memset(buffer, 0, SECTOR_SIZE * num_blocks);
        return nullptr;
    }

    return buffer;
}

/*if you need to write one sector num_blocks = 1*/
bool AtaDriver::ata_write_sector(const uint32 lba, const uint32 num_blocks, const char* buffer, const drive d)
{
    if(!is_drive_exist(d) or buffer == nullptr) return true;

    // Preapare ports and pass command to device
    Port::outb(0x1F6, LBA | DRIVE_REG_CONST_BITS | (d.id << 4) | ((lba >> 24) & 0xF));
    Port::outb(0x1F1, 0x00);
    Port::outb(0x1F2, num_blocks);
    Port::outb(0x1F3, lba & 0xFF);
    Port::outb(0x1F4, (lba >> 8) & 0xFF);
    Port::outb(0x1F5, (lba >> 16) & 0xFF);
    Port::outb(0x1F7, 0x30);

    write_data_port write_port;
    bool result = data_port_do_operation(&write_port, (void *)buffer);

    cache_flush(d);
    return result;
};
