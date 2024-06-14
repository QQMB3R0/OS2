#include "Keyboard.h"
#define BUFFLEN 128
// New characters are added to hd. characters are pulled off of tl.
uint8 kb_buff[BUFFLEN];
uint8 kb_buff_hd;
uint8 kb_buff_tl;
void Keyboard::KB_init()
{
    this->k = 0;
    //Port::outb(0x20, 0x11);
    //Port::outb(0xA0, 0x11);

    //Port::outb(0x21,0x20);
    //Port::outb(0xA1,0x28);

    //Port::outb(0x21,0x00);
    //Port::outb(0xA1,0x00);

    //Port::outb(0x21, 0x01);
    //Port::outb(0xA1, 0x01);

    //Port::outb(0x21, 0xff);
    //Port::outb(0xA1, 0xff);

    Port::outb(0x21, 0xFD);
    
}

void Keyboard::setChar(char c)
{
    this ->k = c;
}

char Keyboard::getChar()
{
    while (1)
    {
        asm volatile("cli\n\t");
        if (kb_buff_hd != kb_buff_tl)
        {
            char c = kb_buff[kb_buff_tl];
            kb_buff_tl = (kb_buff_tl + 1) % BUFFLEN;
            ih_keyboard();
            asm volatile("sti");
            return c;
        }
        asm volatile("sti");
        pause();
    }
    
}

extern "C" void ih_keyboard()
{
    asm volatile("cli\n\t");
    uint8 next_hd = (kb_buff_hd + 1) % BUFFLEN;
    unsigned char status;
	char keycode;
	/* write EOI */
	Port::outb(0x20, 0x20);
    status = Port::inb(KEYBOARD_STATUS_PORT);
    //display <<"keyboard status port: "<<(uint32)status<<'\n';    
    char k;
	/* Lowest bit of status will be set if buffer is not empty */
    if (status & 0x01)
    {
        keycode = Port::inb(KEYBOARD_DATA_PORT);
        if (keycode < 0)
         return;
        display << (const char)keyboard_map[keycode];
        display.setChar(0x07);
        kb_buff[kb_buff_hd] = (const char)keyboard_map[keycode];
        kb_buff_hd = next_hd;
   }
 
    
    
}