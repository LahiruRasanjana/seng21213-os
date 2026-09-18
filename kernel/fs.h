#ifndef FS_H
#define FS_H

#include "../include/types.h"

#define FS_MAX_FILES 16
#define FS_MAX_FILENAME 32
#define FS_MAX_FILE_SIZE 512

typedef struct {
    char name[FS_MAX_FILENAME];
    uint32_t size;
    uint32_t block;
    int used;
} file_t;

void fs_init(void);
int fs_create(const char *name);
int fs_write(const char *name, const char *data);
int fs_read(const char *name, char *buffer);
int fs_delete(const char *name);
void fs_list(void);

#endif
