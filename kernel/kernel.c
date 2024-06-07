#include "GlobalConstruct.h"
#include "../arch/x86/io/GlobalObj.h"
#include "../arch/x86/inc/TypeConverter.h"
#include "../arch/x86/io/Port.h"
#include "../arch/x86/io/Keyboard.h"
#include "../arch/x86/Idt/Idt.h"
void main()
{
    	GlobalConstruct::terminalInit();
    	GlobalConstruct::gdtInit();
	//display << "Hello, World!\n";
	Idt i;
	i.IDTinit();
	asm volatile("sti");
	Keyboard keyboard;
	keyboard.KB_init();
//	asm("int $0x21");
	while(1);
};

