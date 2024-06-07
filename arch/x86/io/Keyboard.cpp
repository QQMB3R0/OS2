#include "Keyboard.h"

void Keyboard::KB_init()
{
    Port::outb(0x21, 0xFD);
}

extern "C" void ih_keyboard()
{
    unsigned char status;
	char keycode;

	/* write EOI */
	Port::outb(0x20, 0x20);
    
	status = Port::inb(KEYBOARD_STATUS_PORT);

	/* Lowest bit of status will be set if buffer is not empty */
    if (status & 0x01)
    {
        keycode = Port::inb(KEYBOARD_DATA_PORT);
        if (keycode < 0)
         return;
        display << keyboard_map[keycode];

   }
 
    
    
}
