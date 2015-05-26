#include <sys/timer.h>
#include <sys/gdt.h>
#include <sys/page.h>
#include <sys/process.h>
//CHANGED

size_t ticks = 0;
extern int isRunning;
extern struct PCB *current_pcb;
extern struct PCB *sleep_pcb_head;
extern struct PCB *wait_pcb_head;
extern struct PCB *pcb_head;
extern struct tss_t tss;
extern int numProcesses; //26th APR: AADY
void init_timer(uint32_t frequency){
	uint32_t divisor = 1193180 / frequency;
	outb(0x43, 0x36);
	unsigned char lower = (unsigned char)(divisor & 0xFF);
    unsigned char upper = (unsigned char)((divisor >> 8) & 0xFF);
	outb(0x40, lower);
   	outb(0x40, upper);
}

void timer_handler(struct regs* r){
	int a;
	int s=0;
	int h=0;
	int m=0;
	int wpid = -1;
	struct PCB *temp = sleep_pcb_head; // AASWAD April 26th
	struct PCB *temp1 = pcb_head; // AASWAD April 26th
	struct PCB *temp2; // AASWAD April 27th
	struct PCB *wtemp = wait_pcb_head; // AASWAD April 30th
	ticks++;
	if(ticks % 100 == 0){
		// AASWAD STARTS
		if (temp != NULL) {
			while (temp1 -> next) temp1 = temp1 -> next;
			while (temp != NULL) {
				temp2 = temp -> next;
				if (--(temp -> sleepTimeLeft) <= 0) {
					if(!(temp -> next) && !(temp -> prev)) sleep_pcb_head = NULL;
					if(temp -> prev) (temp -> prev) -> next = temp -> next;
					if(temp -> next) (temp -> next) -> prev = temp -> prev;
					temp1 -> next = temp;
					temp -> prev = temp1;
					temp -> sleepTimeLeft = 0;
					temp -> next = NULL;
					temp1 = temp;
				}
				temp = temp2;
			}
		}
		// AASWAD ENDS
		s = ticks / 100;
		if(s >= 60){
			m = s/60;
			if(m >= 60){
				h = m/60;
				if(h >= 24){
 					h = h%24;
  				}
				m = m%60;
			}
			s = s%60;
		}
		print_time("%d:%d:%d",h,m,s);	
	}
	outb(0x20,0x20);
	//if((!(ticks % TICKS)) && (isRunning >= 0))
	if(!(ticks % TICKS))
	{
	if (current_pcb != NULL)
	{
		(current_pcb -> time)++;
	}
	
		if(pcb_head)//Some process to run
		{
			//Change this : IMP
			current_pcb->reg.ex_rsp = (uint64_t)r;
			
			if(numProcesses==-1)
				current_pcb=pcb_head;
			else
			{
			if(!current_pcb->next)
				current_pcb = pcb_head;
			else
				current_pcb = current_pcb->next;
			}



			// Aaswad Starts
			if (wtemp != NULL) {
				wpid = current_pcb -> processID;
				temp1 = pcb_head;
				while ((temp1 -> next) != NULL) temp1 = temp1 -> next;
				while (wtemp != NULL) {
					temp2 = wtemp -> next;
					if (wtemp -> waitPID == wpid) {
						if(!(wtemp -> next) && !(wtemp -> prev)) wait_pcb_head = NULL;
						if(wtemp -> prev) (wtemp -> prev) -> next = wtemp -> next;
						if(wtemp -> next) (wtemp -> next) -> prev = wtemp -> prev;
						temp1 -> next = wtemp;
						wtemp -> prev = temp1;
						wtemp -> sleepTimeLeft = 0;
						wtemp -> next = NULL;
						temp1 = wtemp;
					}
					wtemp = temp2;
				}
			}
			// Aaswad Ends
			if(current_pcb -> hasRan++)
			{
				//Change stacks
				loadcr3((void *) current_pcb -> cr3);
				tss.rsp0 = (current_pcb -> ersp);
				__asm__ __volatile__("movq %1, %%rax; \
					movq %%rax, %%rsp; \
					popq %%rsp; \
					popq %%r15; \
					popq %%r14; \
					popq %%r13; \
					popq %%r12; \
					popq %%r11; \
					popq %%r10; \
					popq %%r9; \
					popq %%r8; \
					popq %%rbp; \
					popq %%rdx; \
					popq %%rcx; \
					popq %%rbx; \
					popq %%rax; \
					popq %%rsi; \
					popq %%rdi; \
					add $0x08, %%rsp; \
					sti; \
					iretq;": "=r"(a) :"r"(current_pcb->reg.ex_rsp) : "rax");
			}
			else
			{
				build_process_address_space(current_pcb);

				(current_pcb -> hasRan)++;
				loadcr3((void *) current_pcb -> cr3);
				tss.rsp0 = (current_pcb -> ersp);
				//Change to exception stack of this process, do an iretq from here itself
				__asm__ __volatile__("cli; \
					movq $0x23, %%rax; \
					movq %%rax, %%ds; \
					movq %%rax, %%es; \
					movq %%rax, %%fs; \
					movq %%rax, %%gs; \
					pushq $0x23; \
					movq $0xFFFFFF7F7FFFDFF8, %%rax; \
					pushq %%rax; \
					pushfq; \
					popq %%rax; \
					or $0x200, %%rax; \
					pushq %%rax; \
					pushq $0x1B; \
					movq %1, %%rax; \
					pushq %%rax; \
					iretq;"
					: "=r" (a)
					: "r" ((current_pcb->reg).rip)
					: "rax");
			}	
		}			
		else
		{	
			//Nothing to schedule
			//Do nothing
		}
	}
}
