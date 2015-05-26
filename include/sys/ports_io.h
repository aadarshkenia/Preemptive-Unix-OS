#ifndef _PORTS_IO_H
#define _PORTS_IO_H

#include <sys/defs.h>
void outb(uint16_t, unsigned char);
unsigned char inb(uint16_t); 

#endif
