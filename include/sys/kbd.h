#ifndef KBD_H
#define KBD_H

#include <sys/irq_isr_common.h>

void init_keyboard();
void keyboard_handler(struct regs);

#endif
