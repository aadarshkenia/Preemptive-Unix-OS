#include <sys/irq_isr_common.h>
#include <sys/ports_io.h>
#include <sys/sbunix.h>

void irq_handler(struct regs r){
	if(r.int_number == 32){
		// timer called directly
	}
	else if(r.int_number == 33){
		// keyboard called directly
	}
	else{
		printf("IRQ Common handler\n");
	}

	outb(0x20,0x20);
}
