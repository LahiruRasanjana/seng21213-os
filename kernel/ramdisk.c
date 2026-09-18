#include "ramdisk.h"

static uint8_t ramdisk[RAMDISK_SIZE];

void ramdisk_init(void)
{
    uint32_t i;

    for (i = 0; i < RAMDISK_SIZE; i++) {
        ramdisk[i] = 0;
    }
}

int ramdisk_read(uint32_t block, uint8_t *buffer)
{
    uint32_t i;
    uint32_t offset;

    if (block >= RAMDISK_BLOCKS || buffer == NULL) {
        return -1;
    }

    offset = block * RAMDISK_BLOCK_SIZE;

    for (i = 0; i < RAMDISK_BLOCK_SIZE; i++) {
        buffer[i] = ramdisk[offset + i];
    }

    return 0;
}

int ramdisk_write(uint32_t block, const uint8_t *buffer)
{
    uint32_t i;
    uint32_t offset;

    if (block >= RAMDISK_BLOCKS || buffer == NULL) {
        return -1;
    }

    offset = block * RAMDISK_BLOCK_SIZE;

    for (i = 0; i < RAMDISK_BLOCK_SIZE; i++) {
        ramdisk[offset + i] = buffer[i];
    }

    return 0;
}

