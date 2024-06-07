#define HEAP_H
#ifdef HEAP_H

#include "../inc/types.h"
#include "memory.h"
#include "phys_mem.h"
#include "virt_mem.h"

#define PAGE_SIZE               4096
#define MAX_PAGE_ALIGNED_ALLOCS 32

typedef struct memBlock
{
    uint32 size;
    bool free;
    uint32 v_addr;
    uint32 pcount;
    struct memBlock* next;
} mem_block;

static mem_block *malloc_list_head = 0;    // Start of linked list
static uint32 malloc_virt_address = 0;
static uint32 malloc_phys_address = 0;
static uint32 total_malloc_pages  = 0;
mem_block* kmalloc_list_head = NULL;
mem_block* umalloc_list_head = NULL;
page_directory* kmalloc_dir	= 0;

void merge_free_blocks(mem_block* block){
    mem_block* cur = block;
    while (cur != NULL && cur->next != NULL)
    {
        if (cur->free == true && cur->next->free == true){
                cur->size += cur->next->size + sizeof(mem_block);
                if (cur->next->next!=NULL)
                        cur->next = cur->next->next;
                else{
                    cur->next = NULL;
                    break;
                }
        }
        cur = cur->next;
    }
    
}
void split_block(mem_block* node, const uint32 size){
    mem_block* newnode = (mem_block*)((void*)node + size + sizeof(mem_block));
    newnode->size = node->size - size - sizeof(mem_block);
    newnode->free = true;
    newnode->next = node->next;
    newnode->v_addr = node->v_addr;

    node->size = size;
    node->free = false;
    node->next = newnode;
    node->pcount -= (size / PAGE_SIZE) + 1;
}
void kmalloc_init(const uint32 bytes)
{
		kmalloc_dir = current_page_directory;
		total_malloc_pages = bytes / PAGE_SIZE;
		if (bytes % PAGE_SIZE > 0) total_malloc_pages++;

		malloc_phys_address = (uint32)allocate_blocks(total_malloc_pages);
		malloc_list_head    = (mem_block*)malloc_virt_address;

		for (uint32 i = 0, virt = malloc_virt_address; i < total_malloc_pages; i++, virt += PAGE_SIZE) {
			map_page((void*)(malloc_phys_address + i * PAGE_SIZE), (void*)virt);
			pt_entry* page = get_page(virt);
			SET_ATTRIBUTE(page, PTE_READ_WRITE);
		}

		if (kmalloc_list_head != NULL) {
			kmalloc_list_head->v_addr = malloc_virt_address;
			kmalloc_list_head->pcount = total_malloc_pages;
			kmalloc_list_head->size   = (total_malloc_pages * PAGE_SIZE) - sizeof(mem_block);
			kmalloc_list_head->free   = true;
			kmalloc_list_head->next   = NULL;
		}

}
	void* kmallocp(uint32 v_addr) {
		pt_entry page  = 0;
		uint32* temp = (uint32*)allocate_page(&page);
		map_page((void*)temp, (void*)v_addr);
		SET_ATTRIBUTE(&page, PTE_READ_WRITE);	
	}

	void* kmalloc(const uint32 size) {
		if (size <= 0) return 0;
		if (kmalloc_list_head == NULL) kmalloc_init(size);

		//=============
		// Find a block
		//=============

			merge_free_blocks(kmalloc_list_head);
			mem_block* cur = kmalloc_list_head;
			while (cur->next != NULL) {
				if (cur->free == true) {
					if (cur->size == size) break;
					if (cur->size > size + sizeof(mem_block)) break;
				}
				
				cur = cur->next;
			}

		//=============
		// Find a block
		//=============
		// Work with block
		//=============
		
			if (size == cur->size) cur->free = false;
			else if (cur->size > size + sizeof(mem_block)) split_block(cur, size);
			else {
				//=============
				// Allocate new page
				//=============
				
					uint8 num_pages = 1;
					while (cur->size + num_pages * PAGE_SIZE < size + sizeof(mem_block))
						num_pages++;

					uint32 virt = malloc_virt_address + total_malloc_pages * PAGE_SIZE; // TODO: new pages to new blocks. Don`t mix them to avoid pagedir errors in contswitch
					for (uint8 i = 0; i < num_pages; i++) {
						kmallocp(virt);

						virt += PAGE_SIZE;
						cur->size += PAGE_SIZE;
						total_malloc_pages++;
					}

					split_block(cur, size);

				//=============
				// Allocate new page
				//=============
			}
		
		//=============
		// Work with block
		//=============

		return (void*)cur + sizeof(mem_block);
	}
	void* malloc(uint32 size) {
    void* allocated_memory;
	if (!kmalloc_list_head)
        kmalloc_init(size);

    void* allocated_memory = kmalloc(size);

    return allocated_memory;
}
void free(void* ptr) {
    if (ptr == NULL) return;
	kfree(ptr);
}
void* realloc(void* ptr, uint32 size) {
    void* new_data = NULL;
    if (size) {
        if(!ptr) return malloc(size);

        new_data = malloc(size);
        if(new_data) {
            memcpy(new_data, ptr, size);
            free(ptr);
        }
    }

    return new_data;
}
void kfree(void* ptr)
{
		if (ptr == NULL) return;
		for (mem_block* cur = kmalloc_list_head; cur->next; cur = cur->next) 
			if ((void*)cur + sizeof(mem_block) == ptr && cur->free == false) {
				cur->free = true;
				memset(ptr, 0, cur->size);
				merge_free_blocks(kmalloc_list_head);

				break;
			}
}
#endif