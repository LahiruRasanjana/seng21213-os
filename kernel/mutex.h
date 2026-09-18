#ifndef MUTEX_H
#define MUTEX_H

#include "../include/types.h"

typedef struct {
    int locked;
} mutex_t;

void mutex_init(mutex_t *mutex);
void mutex_lock(mutex_t *mutex);
void mutex_unlock(mutex_t *mutex);

#endif
