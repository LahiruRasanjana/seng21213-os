#include "fs.h"
#include "ramdisk.h"
#include "vga.h"

static file_t files[FS_MAX_FILES];

static int fs_strcmp(const char *a, const char *b)
{
    while (*a && (*a == *b)) {
        a++;
        b++;
    }

    return (uint8_t)*a - (uint8_t)*b;
}

static uint32_t fs_strlen(const char *s)
{
    uint32_t len = 0;

    while (s[len]) {
        len++;
    }

    return len;
}

static void fs_strcpy(char *dest, const char *src)
{
    while (*src) {
        *dest++ = *src++;
    }

    *dest = '\0';
}

void fs_init(void)
{
    int i;

    for (i = 0; i < FS_MAX_FILES; i++) {
        files[i].used = 0;
        files[i].size = 0;
        files[i].block = i;
        files[i].name[0] = '\0';
    }
}

int fs_create(const char *name)
{
    int i;

    for (i = 0; i < FS_MAX_FILES; i++) {
        if (files[i].used &&
            fs_strcmp(files[i].name, name) == 0) {
            return -1;
        }
    }

    for (i = 0; i < FS_MAX_FILES; i++) {
        if (!files[i].used) {
            files[i].used = 1;
            files[i].size = 0;
            fs_strcpy(files[i].name, name);
            return 0;
        }
    }

    return -1;
}

int fs_write(const char *name, const char *data)
{
    int i;
    uint8_t buffer[RAMDISK_BLOCK_SIZE];
    uint32_t len;
    uint32_t j;

    for (i = 0; i < FS_MAX_FILES; i++) {
        if (files[i].used &&
            fs_strcmp(files[i].name, name) == 0) {

            len = fs_strlen(data);

            if (len >= RAMDISK_BLOCK_SIZE) {
                len = RAMDISK_BLOCK_SIZE - 1;
            }

            for (j = 0; j < RAMDISK_BLOCK_SIZE; j++) {
                buffer[j] = 0;
            }

            for (j = 0; j < len; j++) {
                buffer[j] = (uint8_t)data[j];
            }

            files[i].size = len;

            return ramdisk_write(files[i].block, buffer);
        }
    }

    return -1;
}

int fs_read(const char *name, char *buffer)
{
    int i;

    for (i = 0; i < FS_MAX_FILES; i++) {
        if (files[i].used &&
            fs_strcmp(files[i].name, name) == 0) {

            return ramdisk_read(files[i].block, (uint8_t *)buffer);
        }
    }

    return -1;
}

int fs_delete(const char *name)
{
    int i;

    for (i = 0; i < FS_MAX_FILES; i++) {
        if (files[i].used &&
            fs_strcmp(files[i].name, name) == 0) {

            files[i].used = 0;
            files[i].size = 0;
            files[i].name[0] = '\0';

            return 0;
        }
    }

    return -1;
}

void fs_list(void)
{
    int i;

    vga_puts("\n  Files\n");
    vga_puts("  ----------------\n");

    for (i = 0; i < FS_MAX_FILES; i++) {
        if (files[i].used) {
            vga_puts("  ");
            vga_puts(files[i].name);
            vga_puts("\n");
        }
    }

    vga_puts("\n");
}
