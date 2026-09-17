
#include "process.h"
#include "scheduler.h"
#include "vga.h"

pcb_t process_table[MAX_PROCESSES];
pcb_t *current_process = NULL;

static uint32_t next_pid = 1;

void process_init(void)
{
    int i;

    for (i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].pid = 0;
        process_table[i].state = TERMINATED;
        process_table[i].esp = 0;
        process_table[i].eip = 0;
        process_table[i].next = NULL;
    }

    current_process = NULL;
    next_pid = 1;
}

pcb_t *process_create(void (*entry)(void))
{
    int i;

    for (i = 0; i < MAX_PROCESSES; i++) {

        if (process_table[i].state == TERMINATED) {

            pcb_t *process = &process_table[i];

            process->pid = next_pid++;
            process->state = READY;
            process->eip = (uint32_t)entry;

            uint32_t *stack_top =
                &process->stack[STACK_SIZE / 4];
                
            *(--stack_top) = (uint32_t)entry;     /* ret address */
            *(--stack_top) = 0x00000002;          /* EFLAGS */

            *(--stack_top) = 0; /* EAX */     
            *(--stack_top) = 0; /* ECX */
            *(--stack_top) = 0; /* EDX */
            *(--stack_top) = 0; /* EBX */
            *(--stack_top) = 0; /* ESP placeholder */
            *(--stack_top) = 0; /* EBP */
            *(--stack_top) = 0; /* ESI */
            *(--stack_top) = 0; /* EDI */

            process->esp = (uint32_t)stack_top;
            process->next = NULL;

            return process;


        }
    }

    return NULL;
}
void process_yield(void)
{
    schedule();
}

void process_exit(void)
{
    if (current_process != NULL) {
        current_process->state = TERMINATED;
    }

    schedule();

    while (1) {
        __asm__ __volatile__("hlt");
    }
}

void scheduler_tick(void)
{
     schedule();
}
void process_list(void)
{
    int i;

    vga_puts("\n  PID    STATE\n");
    vga_puts("  ----------------\n");

    for (i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state != TERMINATED) {
            vga_printf("  %d      ", process_table[i].pid);

            switch (process_table[i].state) {
                case READY:
                    vga_puts("READY\n");
                    break;

                case RUNNING:
                    vga_puts("RUNNING\n");
                    break;

                case BLOCKED:
                    vga_puts("BLOCKED\n");
                    break;

                case TERMINATED:
                    vga_puts("TERMINATED\n");
                    break;
            }
        }
    }

    vga_puts("\n");
}

