#include "pmm.h"

static uint8_t page_bitmap[TOTAL_PAGES];

static uint32_t free_pages = 0;
static uint32_t used_pages = 0;

void pmm_init(void)
{
    uint32_t i;

    for (i = 0; i < TOTAL_PAGES; i++) {
        page_bitmap[i] = 0;
    }

    /*
     * Reserve the first 1 MB for BIOS, bootloader,
     * kernel-related low memory, and hardware regions.
     */
    for (i = 0; i < (0x100000 / PAGE_SIZE); i++) {
        page_bitmap[i] = 1;
    }

    used_pages = 0x100000 / PAGE_SIZE;
    free_pages = TOTAL_PAGES - used_pages;
}

void *pmm_alloc_page(void)
{
    uint32_t i;

    for (i = 0; i < TOTAL_PAGES; i++) {
        if (page_bitmap[i] == 0) {
            page_bitmap[i] = 1;

            free_pages--;
            used_pages++;

            return (void *)(i * PAGE_SIZE);
        }
    }

    return NULL;
}

void pmm_free_page(void *page)
{
    uint32_t page_number;

    if (page == NULL) {
        return;
    }

    page_number = ((uint32_t)page) / PAGE_SIZE;

    if (page_number < TOTAL_PAGES &&
        page_bitmap[page_number] == 1) {

        page_bitmap[page_number] = 0;

        free_pages++;
        used_pages--;
    }
}

uint32_t pmm_get_free_pages(void)
{
    return free_pages;
}

uint32_t pmm_get_used_pages(void)
{
    return used_pages;
}
