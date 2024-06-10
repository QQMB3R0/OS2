#include "Port.h"

void Port::outb(unsigned short port, unsigned char value)
{
   asm volatile("outb %b0, %w1\n\t"
                :: "a" (value), "Nd" (port) : "memory");
}

byte Port::inb(unsigned short port)
{
   byte res;
   asm volatile("inb %w1, %b0"
                :"=a"(res) : "Nd"(port) : "memory");
   return res;
}

word Port::inw(unsigned short port)
{
   word res;
   asm volatile("inw %w1, %w0"
                :"=a"(res) : "Nd"(port) : "memory");
   return res;
}

void Port::outw(unsigned short port, unsigned int word)
{
   asm volatile("outw %w0, %w1\n\t"
                :: "a" (word), "Nd" (port) : "memory");
}
