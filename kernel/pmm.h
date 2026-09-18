#ifndef PMM_H
#define PMM_H

#include "../include/types.h"

#define PAGE_SIZE 4096
#define TOTAL_MEMORY (16 * 1024 * 1024)
#define TOTAL_PAGES (TOTAL_MEMORY / PAGE_SIZE)

void pmm_init(void);
void *pmm_alloc_page(void);
void pmm_free_page(void *page);

uint32_t pmm_get_free_pages(void);
uint32_t pmm_get_used_pages(void);

#endif
