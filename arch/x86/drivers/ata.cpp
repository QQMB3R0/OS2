#include "ata.h"

void AtaDriver::displayInfo()
{
    display << '\n' << "Data register: " << (uint32)Port::inw(0x1f0) << '\n';
    display << "Error register: " << (uint32)Port::inb(0x1f1) << '\n';
    display << "Sector count register: " << (uint32)Port::inb(0x1f2) << '\n';
    display << "Sector number register: " << (uint32)Port::inb(0x1f3) << '\n';
    display << "Cylinder Low register: " << (uint32)Port::inb(0x1f4) << '\n';
    display << "Cylinder High egister: " << (uint32)Port::inb(0x1f5) << '\n';
    display << "Drive register: " << (uint32)Port::inb(0x1f6) << '\n';
    display << "Status register: " << (uint32)Port::inb(0x1f7) << '\n';
    display << "Alternate status register: " << (uint32)Port::inb(0x3f6) << '\n';
    display << "Drive address register: " << (uint32)Port::inb(0x3f7) << '\n';
};

AtaDriver::AtaDriver()
{
}

/*if you need to read one sector num_blocks = 1*/
uint16 *AtaDriver::ata_read_sector(uint32 lba, uint32 num_blocks)
{
    Port::outb(0x1F6, 0x70 | (lba >> 24) & 0xF);
    Port::outb(0x1F1, 0x00);
    Port::outb(0x1F2, num_blocks);
    Port::outb(0x1F3, lba & 0xFF);
    Port::outb(0x1F4, (lba >> 8) & 0xFF);
    Port::outb(0x1F5, (lba >> 16) & 0xFF);
    Port::outb(0x1F7, 0x20);
    displayInfo();
    uint16* buffer = (uint16*) kmalloc(SECTOR_SIZE * num_blocks);
    if (buffer == NULL)
        return NULL;    
    //wait device
    while(1) {
        uint8 status = Port::inb(0x1F7);
        if(status & STAT_DRQ) {
            // Drive is ready to transfer data!
            break;
        }else if(status & STAT_ERR) {
            display<<"DISK SET ERROR STATUS!";
        }
    }
    memset(buffer, 0, SECTOR_SIZE * num_blocks);    
    for (uint32 i = 0; i < num_blocks; i++)
    {
        for (uint16 i = 0; i < SECTOR_SIZE; i++)
        {
            uint16 value = Port::inw(0x1F0);
            display << (uint32)i << '/';
            buffer[i] = value;
        }
    }
return buffer;
}
/*if you need to write one sector num_blocks = 1*/
int AtaDriver::ata_write_sector(uint32 lba, uint32 num_blocks,const uint16* buffer)
{
    if(lba == 0) return -1;
    int t = 150000;
    while (--t>0)
        continue;
   
    Port::outb(0x1F6, 0x1E0 | (0xF0 << 4) | ((lba >> 24) & 0x0F));
    Port::outb(0x1F1, 0x00);
    Port::outb(0x1F2, num_blocks);
    Port::outb(0x1F3, lba & 0xFF);
    Port::outb(0x1F4, (lba >> 8) & 0xFF);
    Port::outb(0x1F5, (lba >> 16) & 0xFF);
    Port::outb(0x1F7, 0x30);
    if (buffer == NULL)
        return NULL;    
    //wait divace
    while(1) {
        uint8 status = Port::inb(0x1F7);
        if(status & STAT_DRQ) {
            // Drive is ready to transfer data!
            break;
        }else if(status & STAT_ERR) {
            display<<"DISK SET ERROR STATUS!";
        }
    }
    for (uint32 i = 0; i < num_blocks; i++)
    {
        for (int i = 0; i < SECTOR_SIZE; i++)
        {
            uint16 data = buffer[i];
            Port::outw(0x1F0,data);
        }
        buffer += SECTOR_SIZE;
    }
    
    
    return 1;
}
