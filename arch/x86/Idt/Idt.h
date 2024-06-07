#ifndef IDT_H
#define IDT_H
#include "../io/Port.h"
#include "../inc/types.h"
typedef unsigned long size_t;
#define IDTSIZE 0x100
#define INTERRUPT_GATE 0x8E00
#define LOW_WORD(addr) ((addr & 0xffff))
#define HIGH_WORD(addr) ((addr & 0xffff0000) >> 16)
#define GDT_KCODE_SELECTOR 0x08
#define INT_KEYBOARD 0x21
    struct IDT32
    {
        short int low_offset;
        short int selector;
        short int  gateType;
        short int height_offset;         
    }__attribute__((packed));
    /*
        struct IDTR32
    {
        short int limit;
        uint32 base;
    }__attribute__((packed));
    */
class Idt
{
    //static uint64 idtPtr;
    //IDTR32 idtPtr;

private:
    static void setGate(uint8 offset, size_t handler);

public:
    void pic_init();
    void IDTinit();
};
extern "C" void _asm_schedule();
extern "C" void asm_ih_keyboard();
extern "C" void idt_load(size_t* ptr);

/*
extern "C" void isr0();
extern "C" void isr1();
extern "C" void isr2();
extern "C" void isr3();
extern "C" void isr4();
extern "C" void isr5();
extern "C" void isr6();
extern "C" void isr7();
extern "C" void isr8();
extern "C" void isr9();
extern "C" void isr10();
extern "C" void isr11();
extern "C" void isr12();
extern "C" void isr13();
extern "C" void isr14();
extern "C" void isr15();
extern "C" void isr16();
extern "C" void isr17();
extern "C" void isr18();
extern "C" void isr19();
extern "C" void isr20();
extern "C" void isr21();
extern "C" void isr22();
extern "C" void isr23();
extern "C" void isr24();
extern "C" void isr25();
extern "C" void isr26();
extern "C" void isr27();
extern "C" void isr28();
extern "C" void isr29();
extern "C" void isr30();
extern "C" void isr31();
extern "C" void irq0();
extern "C" void irq1();
extern "C" void irq2();
extern "C" void irq3();
extern "C" void irq4();
extern "C" void irq5();
extern "C" void irq6();
extern "C" void irq7();
extern "C" void irq8();
extern "C" void irq9();
extern "C" void irq10();
extern "C" void irq11();
extern "C" void irq12();
extern "C" void irq13();
extern "C" void irq14();
extern "C" void irq15();
*/

#endif
