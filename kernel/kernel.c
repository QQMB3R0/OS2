#include "GlobalConstruct.h"
#include "../arch/x86/io/GlobalObj.h"
#include "../arch/x86/inc/TypeConverter.h"
#include "../arch/x86/io/Port.h"
#include "../arch/x86/io/Keyboard.h"
#include "../arch/x86/Idt/Idt.h"
#include "../arch/x86/drivers/cli.h"
#include "../arch/x86/drivers/ata.h"

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
	cli term;
	term.init_cli();

	AtaDriver ataDriver;
	ataDriver.displayInfo();

	display << "Disk data: ";
	char * buf = (char *)ataDriver.ata_read_sector(1, 1);
	if(buf == nullptr) display << "data empty\n";
	else display << (uint32)*buf;

	// while(1);
};

