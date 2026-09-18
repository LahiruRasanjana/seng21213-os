#ifndef THREAD_H
#define THREAD_H

#include "../include/types.h"

#define MAX_THREADS 16
#define THREAD_STACK_SIZE 4096

typedef enum {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_TERMINATED
} thread_state_t;

typedef struct thread {
    uint32_t tid;
    thread_state_t state;
    uint32_t esp;
    uint32_t stack[THREAD_STACK_SIZE / 4];
    struct thread *next;
} thread_t;

void thread_init(void);
thread_t *thread_create(void (*entry)(void));
void thread_yield(void);
void thread_exit(void);
void thread_start(void);
void thread_list(void);

#endif
