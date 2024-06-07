#include "Port.h"
void Port::outb(unsigned short port, unsigned char value)
{
   write_port(port,value);
}

char Port::inb(unsigned short port)
{
   return read_port(port);
}

unsigned short Port::inw(unsigned short port)
{
    return inw_asm(port);
}

void Port::outw(unsigned short port, unsigned int word)
{
     asm volatile("out %%ax, %%dx" : : "a" (word), "d" (port));
}
