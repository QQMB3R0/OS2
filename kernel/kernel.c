#include "GlobalConstruct.h"
#include "../arch/x86/io/GlobalObj.h"
#include "../arch/x86/inc/TypeConverter.h"
#include "../arch/x86/io/Port.h"
#include "../arch/x86/io/Keyboard.h"
#include "../arch/x86/Idt/Idt.h"
#include "../arch/x86/drivers/cli.h"
#include "../arch/x86/drivers/ata.h"
#include "../arch/x86/inc/types.h"
#include "multiboot.h"
// symbols from linker.ld for section addresses
 uint8 __kernel_section_start;
 uint8 __kernel_section_end;
 uint8 __kernel_text_section_start;
 uint8 __kernel_text_section_end;
 uint8 __kernel_data_section_start;
 uint8 __kernel_data_section_end;
 uint8 __kernel_rodata_section_start;
 uint8 __kernel_rodata_section_end;
 uint8 __kernel_bss_section_start;
 uint8 __kernel_bss_section_end;


typedef struct {
    struct {
        uint32 k_start_addr;
        uint32 k_end_addr;
        uint32 k_len;
        uint32 text_start_addr;
        uint32 text_end_addr;
        uint32 text_len;
        uint32 data_start_addr;
        uint32 data_end_addr;
        uint32 data_len;
        uint32 rodata_start_addr;
        uint32 rodata_end_addr;
        uint32 rodata_len;
        uint32 bss_start_addr;
        uint32 bss_end_addr;
        uint32 bss_len;
    } kernel;

    struct {
        uint32 total_memory;
    } system;

    struct {
        uint32 start_addr;
        uint32 end_addr;
        uint32 size;
    } available;
} KERNEL_MEMORY_MAP;

KERNEL_MEMORY_MAP g_kmap;

int get_kernel_memory_map(KERNEL_MEMORY_MAP *kmap, MULTIBOOT_INFO *mboot_info) {
    uint32 i;

    if (kmap == NULL) return -1;
    kmap->kernel.k_start_addr = (uint32)&__kernel_section_start;
    kmap->kernel.k_end_addr = (uint32)&__kernel_section_end;
    kmap->kernel.k_len = ((uint32)&__kernel_section_end - (uint32)&__kernel_section_start);

    kmap->kernel.text_start_addr = (uint32)&__kernel_text_section_start;
    kmap->kernel.text_end_addr = (uint32)&__kernel_text_section_end;
    kmap->kernel.text_len = ((uint32)&__kernel_text_section_end - (uint32)&__kernel_text_section_start);

    kmap->kernel.data_start_addr = (uint32)&__kernel_data_section_start;
    kmap->kernel.data_end_addr = (uint32)&__kernel_data_section_end;
    kmap->kernel.data_len = ((uint32)&__kernel_data_section_end - (uint32)&__kernel_data_section_start);

    kmap->kernel.rodata_start_addr = (uint32)&__kernel_rodata_section_start;
    kmap->kernel.rodata_end_addr = (uint32)&__kernel_rodata_section_end;
    kmap->kernel.rodata_len = ((uint32)&__kernel_rodata_section_end - (uint32)&__kernel_rodata_section_start);

    kmap->kernel.bss_start_addr = (uint32)&__kernel_bss_section_start;
    kmap->kernel.bss_end_addr = (uint32)&__kernel_bss_section_end;
    kmap->kernel.bss_len = ((uint32)&__kernel_bss_section_end - (uint32)&__kernel_bss_section_start);

    kmap->system.total_memory = mboot_info->mem_low + mboot_info->mem_high;

    for (i = 0; i < mboot_info->mmap_length; i += sizeof(MULTIBOOT_MEMORY_MAP)) {
        MULTIBOOT_MEMORY_MAP *mmap = (MULTIBOOT_MEMORY_MAP *)(mboot_info->mmap_addr + i);
        if (mmap->type != MULTIBOOT_MEMORY_AVAILABLE) continue;
        // make sure kernel is loaded at 0x100000 by bootloader(see linker.ld)
        if (mmap->addr_low == kmap->kernel.text_start_addr) {
            // set available memory starting from end of our kernel, leaving 1MB size for functions exceution
            kmap->available.start_addr = kmap->kernel.k_end_addr + 1024 * 1024;
            kmap->available.end_addr = mmap->addr_low + mmap->len_low;
            // get availabel memory in bytes
            kmap->available.size = kmap->available.end_addr - kmap->available.start_addr;
            return 0;
        }
    }

    return -1;
}

void empty(){};
void main(unsigned long magic,unsigned long addr)
{
	MULTIBOOT_INFO *mboot_info;
    	GlobalConstruct::terminalInit();
    	GlobalConstruct::gdtInit();
	//display << "Hello, World!\n";
	Idt i;
	i.IDTinit();
	asm volatile("sti");
	mboot_info = (MULTIBOOT_INFO *)addr;
	if (get_kernel_memory_map(&g_kmap, mboot_info) < 0) {
        display<<"error: failed to get kernel memory map\n";
        return;
    }
        // put the memory bitmap at the start of the available memory
    	pmm_init(g_kmap.available.start_addr, g_kmap.available.size);
	pmm_init_region(g_kmap.available.start_addr, PMM_BLOCK_SIZE * 256);

    	// initialize heap 20 blocks(81920 bytes)
    	void *start = pmm_alloc_blocks(20);
    	void *end = start + (pmm_next_free_frame(1) * PMM_BLOCK_SIZE);
    	kheap_init(start, end);
	Keyboard keyboard;
	keyboard.KB_init();

//	cli term;
//	term.init_cli();

	AtaDriver ataDriver;

	char buffer[2048] = "WORKING!!\n";
	ataDriver.ata_write_sector(0, 1, buffer);

	display << "Disk data: ";
	uint16_s *buf = (uint16_s *)ataDriver.ata_read_sector(0, 1);
	if(buf == nullptr) display << "data empty\n";
	else display << (char *)buf;
	empty();

	while(1);
};

