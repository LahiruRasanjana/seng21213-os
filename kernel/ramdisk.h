#ifndef RAMDISK_H
#define RAMDISK_H

#include "../include/types.h"

#define RAMDISK_SIZE (64 * 1024)
#define RAMDISK_BLOCK_SIZE 512
#define RAMDISK_BLOCKS (RAMDISK_SIZE / RAMDISK_BLOCK_SIZE)

void ramdisk_init(void);

int ramdisk_read(uint32_t block, uint8_t *buffer);
int ramdisk_write(uint32_t block, const uint8_t *buffer);

#endif

