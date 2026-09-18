#include "vmm.h"
#include "pmm.h"

#define PAGE_DIRECTORY_ENTRIES 1024
#define PAGE_TABLE_ENTRIES 1024

static uint32_t page_directory[PAGE_DIRECTORY_ENTRIES]
    __attribute__((aligned(4096)));

static uint32_t first_page_table[PAGE_TABLE_ENTRIES]
    __attribute__((aligned(4096)));

void vmm_init(void)
{
    uint32_t i;

    for (i = 0; i < PAGE_DIRECTORY_ENTRIES; i++) {
        page_directory[i] = 0;
    }

    for (i = 0; i < PAGE_TABLE_ENTRIES; i++) {
        first_page_table[i] = (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
    }

    page_directory[0] =
        ((uint32_t)first_page_table) | PAGE_PRESENT | PAGE_RW;
}

void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr)
{
    uint32_t page_index;

    page_index = virtual_addr / PAGE_SIZE;

    if (page_index < PAGE_TABLE_ENTRIES) {
        first_page_table[page_index] =
            (physical_addr & 0xFFFFF000) | PAGE_PRESENT | PAGE_RW;
    }
}

uint32_t vmm_get_mapping(uint32_t virtual_addr)
{
    uint32_t page_index;

    page_index = virtual_addr / PAGE_SIZE;

    if (page_index < PAGE_TABLE_ENTRIES) {
        return first_page_table[page_index] & 0xFFFFF000;
    }

    return 0;
}
