#include "semaphore.h"
#include "thread.h"

void semaphore_init(semaphore_t *sem, int value)
{
    sem->value = value;
}

void semaphore_wait(semaphore_t *sem)
{
    while (sem->value <= 0) {
        thread_yield();
    }

    sem->value--;
}

void semaphore_signal(semaphore_t *sem)
{
    sem->value++;
}
