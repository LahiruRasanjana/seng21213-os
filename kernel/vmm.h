#ifndef VMM_H
#define VMM_H

#include "../include/types.h"

#define PAGE_PRESENT 0x1
#define PAGE_RW      0x2

void vmm_init(void);
void vmm_map_page(uint32_t virtual_addr, uint32_t physical_addr);
uint32_t vmm_get_mapping(uint32_t virtual_addr);

#endif
