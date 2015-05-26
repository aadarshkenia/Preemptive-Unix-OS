#include <sys/defs.h>
#include <sys/ports_io.h>

void outb(uint16_t port,unsigned char val){
	__asm__ __volatile__("outb %0, %1" : : "a"(val) , "Nd"(port));
}

unsigned char inb(uint16_t port){
	unsigned char val;
	__asm__ __volatile__("inb %1, %0" : "=a"(val) : "dN"(port));
	return val;
}
