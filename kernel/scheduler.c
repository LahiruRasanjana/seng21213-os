#include "scheduler.h"
#include "process.h"

extern pcb_t process_table[MAX_PROCESSES];
static uint32_t kernel_esp = 0;
extern pcb_t *current_process;

void scheduler_init(void)
{
    current_process = NULL;
}

void scheduler_start(void)
{
    int i;

    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == READY) {
            current_process = &process_table[i];
            current_process->state = RUNNING;

            context_switch(&kernel_esp, current_process->esp);
            return;
        }
    }
}

void schedule(void)
{
    int start = 0;
    int i;
    pcb_t *previous = current_process;

    if (current_process != NULL) {
        if (current_process->state == RUNNING) {
            current_process->state = READY;
        }

        start = (int)(current_process - process_table) + 1;
    }

    for (i = 0; i < MAX_PROCESSES; i++) {
        int index = (start + i) % MAX_PROCESSES;

        if (process_table[index].state == READY) {
            current_process = &process_table[index];
            current_process->state = RUNNING;

            if (previous != NULL && previous != current_process) {
                context_switch(&previous->esp, current_process->esp);
            }

            return;
        }
    }

    current_process = previous;
}
