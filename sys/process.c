#include <sys/sbunix.h>
#include <sys/page.h>
#include <sys/structs.h>
#include <string.h>
#include <sys/process.h>
#include <sys/elf.h>
#include <sys/gdt.h>
//CHANGED

extern uint64_t * kernel_pml4;
extern struct tss_t tss;
extern int numProcesses;
extern int global_counter;
extern struct PCB *current_pcb ;
extern struct PCB *pcb_head;

//void switch_to_user_space(uint64_t entry);
void add_stack_vma(struct PCB* process);
void add_heap_vma(struct PCB* process);

void build_process_address_space(struct PCB* process)
{
	
	uint64_t entry = 0;
	uint64_t e_rsp = UXSTACKTOP;
	struct page* page_temp;
	//struct vma* vma_temp;
	//int ab=0;


	//Allocate for user pml4 table and initialize it
	//Since its in user space, will already be mapped.
	if(!process->pml4)
	{
		page_temp = page_alloc();
		if(page_temp->ref_count==1)
			printf("PANIC\n");
		memset((void *)getVA(page_temp),0,PAGE_SIZE);
		process->cr3 = (uint64_t *)getPA(page_temp);
		process->pml4 = (uint64_t *)getVA(page_temp); 

		
		//Copy 1 entry from kernel_pml4 to this pml4
		*(process->pml4 + PML4OFF(KERNBASE)) = *(kernel_pml4 + PML4OFF(KERNBASE));	
	}
	//printf("K: %x , U: %x\n", *(kernel_pml4 + PML4OFF(KERNBASE)), *(process->pml4 + PML4OFF(KERNBASE)));


	//New cr3 
	//loadcr3((void *)process->cr3);

	//ELF
	entry = getELF(process -> fileName, process);
   	//printf("entry = %x\n", entry);	
	(process -> reg).rip = entry;

	add_heap_vma(process);
	add_stack_vma(process);
	
	//AADY: 30 Apr
	if(process->processID < 0)
	{
		process -> processID = generate_pid(); 
		e_rsp = UXSTACKTOP - (process->processID * PAGE_SIZE);
		(process -> ersp) = e_rsp;
		process -> hasRan = 0; //Already done in pcb_alloc, a safety check here.
	}
	
	
	
	/*
	//TEMP: 26th APR
	//ASM to set up tss
        __asm__ __volatile__("movq %%rsp, %0; \
                movq $0x28,%%rax; \
                ltr %%ax;"
                : "=r" (tss.rsp0)
                :
                : "rax");	
	
	//tss.rsp0 = process->ersp;
	
	//ASM to set up tss    
        __asm__ __volatile__("movq $0x28,%%rax; \
                ltr %%ax;"
                : 
                :
                : "rax");
	
        //printf("tss setting: %x\n",tss.rsp0);
	*/
	
	//switch_to_user_space(entry);		
	
}
//Find the last vma of this process.
struct vma* findLastVMA(struct PCB* process)
{
        struct vma* temp = process->first_vma;
        if(temp==NULL)
                return NULL;
        while(temp->next != NULL)
                temp=temp->next;

        return temp;
}


void add_stack_vma(struct PCB* process)
{
	//STACK
	struct vma* vma_temp=findLastVMA(process);
	struct vma* vma_new=vma_alloc();
	if(vma_new)
	{
		vma_temp->next = vma_new;
		vma_new->start = USTACKTOP-PAGE_SIZE;
		vma_new->end = USTACKTOP;
		vma_new->flags_vma = VM_READ | VM_WRITE; //CHECK? what about execute?
		vma_new->next=NULL;
		vma_new->prev=vma_temp;
	}	
	
}

void add_heap_vma(struct PCB* process)
{
	//HEAP
	struct vma* vma_temp=findLastVMA(process);
	struct vma* vma_new=vma_alloc();
	vma_temp->next=vma_new;
	vma_new->prev=vma_temp;	
	
	vma_new->start = UHEAPSTART;
	vma_new->end = UHEAPSTART;//This denotes current end of heap seg
	vma_new->flags_vma = VM_READ | VM_WRITE;
	vma_new->next=NULL;

}

int generate_pid()
{
	numProcesses++;
	return ++global_counter;	
}

//4th May: AADY
//Process creation: pass 1 as second param is a foreground process
void create_process(char *filename, int foreground)
{
	struct PCB* temp = pcb_alloc();
	struct PCB* last = pcb_head;


	if(temp)
	{
		if(!strcmp(filename, "bin/ideal"))
		{
			if(!pcb_head)
			{
				temp->next=NULL;
				temp->prev=NULL;
				pcb_head = temp;
				current_pcb=temp;
			}

		}
		else
		{
		while(last->next)
		{
			last=last->next;
		}
		last->next=temp;
		temp->prev=last;
		temp->next=NULL;
		}

		
		/*
		else
		{
			current_pcb->next = temp;
			temp->prev = current_pcb;
			current_pcb=temp;
			temp->next=NULL;
		}
		*/

		
		//Initialization to NULL
		temp->fileName = (char *)kmalloc(128);//plus one for null char at last
		strcpy(temp->fileName, filename);
		temp->cwd = (char *)kmalloc(128);
		strcpy(temp->cwd, "/");
		temp->pml4 = NULL; 
		temp->cr3 = NULL;
		temp->processID=-1;
		temp->parentProcessID=-1;
		temp->first_vma = NULL;
		temp->hasRan = 0;
		temp->ersp=0;
		temp->sleepTimeLeft=0;
		temp->time=0;
		temp -> waitPID = -2;
		for(int i=0; i<MAX_FILE_DESCRIPTORS_PER_PROCESS;i++){
			if(i==0 || i==1 || i==2){
				struct file_descriptor *fd = (struct file_descriptor *)kmalloc(sizeof(struct file_descriptor));
		 		fd->fdType = STDFILETYPE;
		 		fd->pipe_ptr = NULL;
				fd->offset = 0;
				fd->perm = 0;
				fd->file_ptr = NULL;
				fd->refCount = 1;
				temp->fileDescriptors[i] = fd;
			}else{
				temp->fileDescriptors[i]=NULL;
			}
		}
		
		//NEW 7th MAY: AADY
		temp->hasExited=0;		
		temp->parentPCB=NULL;
		
		if(foreground)
			temp->isFore = 1;
		else
			temp->isFore = 0;

		//File descriptors?:KAV

	}
	else
	{
		printf("Cannot create more processes");
	}
}
/*
void switch_to_user_space(uint64_t entry)
{
        int ab = 0;
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
        : "=r" (ab)
        : "r" (entry)
        : "rax");

}
*/	
