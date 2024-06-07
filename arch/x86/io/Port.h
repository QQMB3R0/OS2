#ifndef PORT_H
#define PORT_H
class Port
{
public:
    static void outb(unsigned short port, unsigned char value);
    static char inb(unsigned short port);
    static unsigned short inw(unsigned short port);
    static void outw(unsigned short port, unsigned int word);
};
extern "C" void write_port(unsigned short port, unsigned char data);
extern "C" char read_port(unsigned short port); 
extern "C" unsigned short inw_asm( unsigned short port );
#endif