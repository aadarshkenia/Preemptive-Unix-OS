#ifndef _TIMER_H
#define _TIMER_H
#define TICKS 240 //Temp change

#include <sys/sbunix.h>
#include <sys/defs.h>
#include <sys/ports_io.h>
#include <sys/structs.h>

void init_timer(uint32_t);
void timer_handler(struct regs *);

#endif
