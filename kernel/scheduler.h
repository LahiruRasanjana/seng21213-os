#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

void scheduler_init(void);
void schedule(void);
void scheduler_start(void);
void context_switch(uint32_t *old_esp, uint32_t new_esp);

#endif
