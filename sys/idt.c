#include <sys/idt.h>
#include <string.h>
#include <sys/ports_io.h>

extern void isr0();
extern void isr13();
extern void isr14();
extern void isr128();
extern void irq0();
extern void irq1();

#define MAX_IDT 256

struct idt_entry{
	uint16_t offset_low;
	uint16_t selector;
	unsigned char alwaysZero;
	unsigned char flags;
	uint16_t offset_middle;
	uint32_t offset_high;
	uint32_t unused;
}__attribute__((packed));

struct idt_pointer{
	uint16_t limit;
	uint64_t base;
}__attribute__((packed));

struct idt_entry idtEntry[MAX_IDT];
struct idt_pointer idtPtr;

extern void _x86_64_asm_lidt(struct idt_pointer *idtPtr);

void idt_set_gate(unsigned char i, uint64_t offset, uint16_t selector, unsigned char flags){
	idtEntry[i].offset_low = offset & 0xFFFF;
	idtEntry[i].offset_middle = (offset >> 16) & 0xFFFF;
	idtEntry[i].offset_high = (offset >> 32) & 0xFFFFFFFF;
	idtEntry[i].selector = selector;
	idtEntry[i].alwaysZero = 0x0;
	idtEntry[i].flags = flags;
}

void init_idt()
{
	idtPtr.limit = (sizeof(struct idt_entry) * MAX_IDT); //- 1;
	idtPtr.base = (uint64_t) idtEntry;
	memset(&idtEntry,0,(sizeof(struct idt_entry) * MAX_IDT));
	
	//isr
	idt_set_gate(0,(uint64_t)isr0,0x08, 0x8E);
	idt_set_gate(13,(uint64_t)isr13,0x08, 0x8E);
	idt_set_gate(14,(uint64_t)isr14,0x08, 0x8E);
	idt_set_gate(128,(uint64_t)isr128,0x08, 0xEE);
	//irq
   	idt_set_gate(32,(uint64_t)irq0,0x08,0x8E);
   	idt_set_gate(33,(uint64_t)irq1,0x08,0x8E);
	
	_x86_64_asm_lidt(&idtPtr);
}

void init_pic(){

	outb(0x20, 0x11);
  	outb(0xA0, 0x11);
  	outb(0x21, 0x20);
  	outb(0xA1, 0x28);
	outb(0x21, 0x04);
  	outb(0xA1, 0x02);
  	outb(0x21, 0x01);
  	outb(0xA1, 0x01);
  	outb(0x21, 0x0);
  	outb(0xA1, 0x0); 
//  	__asm__("sti;");
}
