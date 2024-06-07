#include "Idt.h"
static struct IDT32 idtEntries[IDTSIZE];

void Idt::setGate(uint8 offset, size_t handler)
{
    idtEntries[offset].low_offset = LOW_WORD(handler);
    idtEntries[offset].selector = GDT_KCODE_SELECTOR;
    idtEntries[offset].gateType = INTERRUPT_GATE;
    idtEntries[offset].height_offset = HIGH_WORD(handler);
}

void Idt::pic_init()
{
    Port::outb(0x20, 0x11);
    Port::outb(0xA0, 0x11);

    Port::outb(0x21,0x20);
    Port::outb(0xA1,0x28);

    Port::outb(0x21,0x00);
    Port::outb(0xA1,0x00);

    Port::outb(0x21, 0x01);
    Port::outb(0xA1, 0x01);

    Port::outb(0x21, 0xff);
    Port::outb(0xA1, 0xff);
}

void Idt::IDTinit()
{
    size_t idtAdrr;
    size_t idt_ptr[2];
    setGate(33, (size_t) asm_ih_keyboard);    
    pic_init();
    idtAdrr = (size_t)((void *)(&idtEntries));
    idt_ptr[0] = (LOW_WORD(idtAdrr)<<16) + (sizeof(struct IDT32) * IDTSIZE - 1);
    idt_ptr[1] = idtAdrr >> 16;
    idt_load(idt_ptr);
}
