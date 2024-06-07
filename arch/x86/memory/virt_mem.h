#define VIRT_MEM_H
#ifdef VIRT_MEM_H

#include "../inc/types.h"
#include "phys_mem.h"
#include "memory.h"
#define PAGES_PER_TABLE 1024
#define TABLES_PER_DIRECTORY 1024

#define PD_INDEX(address) ((address) >> 22)
#define SET_ATTRIBUTE(entry, attr) (*entry |= attr)
#define SET_FRAME(entry, address) (*entry = (*entry & ~0x7FFFF000) | address)   // Only set address/frame, not flags
#define PAGE_PHYS_ADDRESS(dir_entry) ((*dir_entry) & ~0xFFF)    // Clear lowest 12 bits, only return frame/address
#define PT_INDEX(address) (((address) >> 12) & 0x3FF) // Max index 1023 = 0x3FF

typedef uint32 pd_entry;  // Page directory entry
typedef uint32 pt_entry;  // Page table entry
typedef uint32 physical_address; 
typedef uint32 virtual_address; 

// Page directory: handle 4GB each, 1024 page tables * 4MB
typedef struct {
    pd_entry entries[TABLES_PER_DIRECTORY];
} page_directory;

// Page table: handle 4MB each, 1024 entries * 4096
typedef struct {
    pt_entry entries[PAGES_PER_TABLE];
} page_table;

typedef enum {
    PTE_PRESENT       = 0x01,
    PTE_READ_WRITE    = 0x02,
    PTE_USER          = 0x04,
    PTE_WRITE_THROUGH = 0x08,
    PTE_CACHE_DISABLE = 0x10,
    PTE_ACCESSED      = 0x20,
    PTE_DIRTY         = 0x40,
    PTE_PAT           = 0x80,
    PTE_GLOBAL        = 0x100,
    PTE_FRAME         = 0x7FFFF000,   // bits 12+
} PAGE_TABLE_FLAGS;

typedef enum {
    PDE_PRESENT       = 0x01,
    PDE_READ_WRITE    = 0x02,
    PDE_USER          = 0x04,
    PDE_WRITE_THROUGH = 0x08,
    PDE_CACHE_DISABLE = 0x10,
    PDE_ACCESSED      = 0x20,
    PDE_DIRTY         = 0x40,          // 4MB entry only
    PDE_PAGE_SIZE     = 0x80,          // 0 = 4KB page, 1 = 4MB page
    PDE_GLOBAL        = 0x100,         // 4MB entry only
    PDE_PAT           = 0x2000,        // 4MB entry only
    PDE_FRAME         = 0x7FFFF000,    // bits 12+
} PAGE_DIR_FLAGS;

page_directory *current_page_directory = 0;

// Map a page
bool map_page(void *phys_address, void *virt_address)
{
    // Get page
    page_directory *pd = current_page_directory;

    // Get page table
    pd_entry *entry = &pd->entries[PD_INDEX((uint32)virt_address)];

    // TODO: Use TEST_ATTRIBUTE for this check?
    if ((*entry & PTE_PRESENT) != PTE_PRESENT) {
        // Page table not present allocate it
        page_table *table = (page_table *)allocate_blocks(1);
        if (!table) return false;   // Out of memory

        // Clear page table
        memset(table, 0, sizeof(page_table));

        // Create new entry
        pd_entry *entry = &pd->entries[PD_INDEX((uint32)virt_address)];

        // Map in the table & enable attributes
        SET_ATTRIBUTE(entry, PDE_PRESENT);
        SET_ATTRIBUTE(entry, PDE_READ_WRITE);
        SET_FRAME(entry, (physical_address)table);
    }
}
pt_entry *get_page(const virtual_address address)
{
    // Get page directory
    page_directory *pd = current_page_directory; 

    // Get page table in directory
    pd_entry   *entry = &pd->entries[PD_INDEX(address)];
    page_table *table = (page_table *)PAGE_PHYS_ADDRESS(entry);

    // Get page in table
    pt_entry *page = &table->entries[PT_INDEX(address)];
    
    // Return page
    return page;
}
void* allocate_page(pt_entry* page) {
    void* block = allocate_blocks(1);
    if (block) {
        SET_FRAME(page, (physical_address)block);
        SET_ATTRIBUTE(page, PTE_PRESENT);
    } else {
        display<<"Page allocation error!\n";
        return NULL;
    }

    return block;
}
#endif