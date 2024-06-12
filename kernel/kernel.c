#include "GlobalConstruct.h"
#include "../arch/x86/io/GlobalObj.h"
#include "../arch/x86/inc/TypeConverter.h"
#include "../arch/x86/io/Port.h"
#include "../arch/x86/io/Keyboard.h"
#include "../arch/x86/Idt/Idt.h"
#include "../arch/x86/drivers/cli.h"
#include "../arch/x86/drivers/ata.h"

void empty(){};
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

	char buffer[2048] = "Qwe\n";
	ataDriver.ata_write_sector(0, 1, buffer);

	display << "Disk data: ";
	uint16_s *buf = (uint16_s *)ataDriver.ata_read_sector(0, 1);
	if(buf == nullptr) display << "data empty\n";
	else display << (char *)buf;
	empty();

	while(1);
};

