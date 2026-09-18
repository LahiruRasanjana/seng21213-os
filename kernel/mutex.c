#include "mutex.h"
#include "thread.h"

void mutex_init(mutex_t *mutex)
{
    mutex->locked = 0;
}

void mutex_lock(mutex_t *mutex)
{
    while (mutex->locked) {
        thread_yield();
    }

    mutex->locked = 1;
}

void mutex_unlock(mutex_t *mutex)
{
    mutex->locked = 0;
}
