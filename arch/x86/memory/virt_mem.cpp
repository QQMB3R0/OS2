#include"virt_mem.h"


page_directory* current_page_directory;
page_directory* kernel_page_directory;
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
        //display<<"Page allocation error!\n";
        return NULL;
    }

    return block;
}
