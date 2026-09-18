#include "thread.h"
#include "scheduler.h"
#include "vga.h"

thread_t thread_table[MAX_THREADS];
thread_t *current_thread = NULL;

static uint32_t next_tid = 1;
static uint32_t thread_kernel_esp = 0;

void thread_init(void)
{
    int i;

    for (i = 0; i < MAX_THREADS; i++) {
        thread_table[i].tid = 0;
        thread_table[i].state = THREAD_TERMINATED;
        thread_table[i].esp = 0;
        thread_table[i].next = NULL;
    }

    current_thread = NULL;
    next_tid = 1;
}

thread_t *thread_create(void (*entry)(void))
{
    int i;

    for (i = 0; i < MAX_THREADS; i++) {

        if (thread_table[i].state == THREAD_TERMINATED) {

            thread_t *thread = &thread_table[i];

            thread->tid = next_tid++;
            thread->state = THREAD_READY;

            uint32_t *stack_top =
                &thread->stack[THREAD_STACK_SIZE / 4];

            /* Initial context expected by context_switch */
            *(--stack_top) = (uint32_t)entry;
            *(--stack_top) = 0x00000002;

            *(--stack_top) = 0; /* EAX */
            *(--stack_top) = 0; /* ECX */
            *(--stack_top) = 0; /* EDX */
            *(--stack_top) = 0; /* EBX */
            *(--stack_top) = 0; /* ESP placeholder */
            *(--stack_top) = 0; /* EBP */
            *(--stack_top) = 0; /* ESI */
            *(--stack_top) = 0; /* EDI */

            thread->esp = (uint32_t)stack_top;
            thread->next = NULL;

            return thread;
        }
    }

    return NULL;
}
void thread_start(void)
{
    int i;

    for (i = 0; i < MAX_THREADS; i++) {
        if (thread_table[i].state == THREAD_READY) {
            current_thread = &thread_table[i];
            current_thread->state = THREAD_RUNNING;

            context_switch(&thread_kernel_esp, current_thread->esp);
            return;
        }
    }
}

void thread_yield(void)
{
    int start;
    int i;
    thread_t *previous;

    if (current_thread == NULL) {
        return;
    }

    previous = current_thread;

    if (previous->state == THREAD_RUNNING) {
        previous->state = THREAD_READY;
    }

    start = (int)(previous - thread_table) + 1;

    for (i = 0; i < MAX_THREADS; i++) {
        int index = (start + i) % MAX_THREADS;

        if (thread_table[index].state == THREAD_READY) {
            current_thread = &thread_table[index];
            current_thread->state = THREAD_RUNNING;

            if (current_thread != previous) {
                context_switch(&previous->esp, current_thread->esp);
            }

            return;
        }
    }

    previous->state = THREAD_RUNNING;
}

void thread_exit(void)
{
    if (current_thread != NULL) {
        current_thread->state = THREAD_TERMINATED;
    }

    thread_yield();

    while (1) {
        __asm__ __volatile__("hlt");
    }
}

void thread_list(void)
{
    int i;

    vga_puts("\n  TID    STATE\n");
    vga_puts("  ----------------\n");

    for (i = 0; i < MAX_THREADS; i++) {
        if (thread_table[i].state != THREAD_TERMINATED) {

            vga_printf("  %d      ", thread_table[i].tid);

            switch (thread_table[i].state) {
                case THREAD_READY:
                    vga_puts("READY\n");
                    break;

                case THREAD_RUNNING:
                    vga_puts("RUNNING\n");
                    break;

                case THREAD_BLOCKED:
                    vga_puts("BLOCKED\n");
                    break;

                case THREAD_TERMINATED:
                    vga_puts("TERMINATED\n");
                    break;
            }
        }
    }

    vga_puts("\n");
}
