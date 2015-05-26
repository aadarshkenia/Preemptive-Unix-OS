#ifndef _IDT_H
#define _IDT_H

# include <sys/defs.h>

void init_pic();
void init_idt();
void idt_set_gate(unsigned char i, uint64_t offset, uint16_t selector, unsigned char flags);

#endif
