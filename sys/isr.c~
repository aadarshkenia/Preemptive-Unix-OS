#include <sys/irq_isr_common.h>
#include <sys/sbunix.h>
#include <sys/page.h>
#include <string.h>
#include <sys/structs.h>
#include <sys/elf.h>
#include <sys/tarfs.h>

extern struct PCB *current_pcb;
extern struct PCB *pcb_head;
struct vma* findVMA(uint64_t addr,int errno);//Finds vma corresponding to faulting VMA
void allocatePages(struct vma* vma);
void copyELF(struct vma* vmas);
void copy_on_write_handler(uint64_t addr);


int KillisCalled(struct PCB *head, uint64_t killPID);

void isr_handler(struct regs r)
{
    if(r.int_number == 0)
    {
        printf("Exception : Divide by Zero\n");
        printf("Faulting instruction: %x\n",readcr2());
        KillisCalled(pcb_head, current_pcb->processID);
        while(1);
//      exit(0);
    }
    else if(r.int_number == 13)
    {
        printf("Exception : General Protection fault\n");
        printf("Faulting instruction: %x\n",readcr2());
        printf("Error code: %x\n",r.err_code);
        while(1);
    }
    else if(r.int_number == 14)
    {
    	
        // printf("Page fault\t");
        // printf("Fault addr: %x\t",readcr2());
        // printf("Error code: %x\n",r.err_code);
		

	//while(1);//test 20th apr
        
        // New page fault handler
        
        uint64_t errorCode = r.err_code;
        uint64_t faultAddr = (uint64_t)readcr2();
        struct vma *vmas = current_pcb->first_vma;

    	if(!(errorCode & 0x4))//Page fault in kernel mode
    	{
		/*
		//allocate to heap if malloc ed
		if(faultAddr>=UHEAPSTART && faultAddr<=UHEAPLIMIT-PAGE_SIZE)
		{
			page_insert(current_pcb->pml4,alignDown((void *)faultAddr), page_alloc(), BIT_USER | BIT_RW | BIT_PRESENT);
			printf("Inserted page for heap mem in kernel space\n");
		}
		
		else
		{
		*/
    			printf("Page fault in kernel mode.\n");
    			
		//}
    	}	


	//AADY: 26th APR
	if(errorCode & 0x1) //Page present but page protection violation, check for COW
	{
		if(errorCode & 0x2)//If page fault on write
		{
			copy_on_write_handler(faultAddr);
		}
		return;
	}


    	//Find VMA corresponding to faulting address
    	vmas = findVMA(faultAddr, errorCode);
    	if(!vmas)//Illegal access
    	{
    		printf("Segmentation fault.\n");
    		//KILL PROCESS
    		KillisCalled(pcb_head, current_pcb->processID);

 		while(1);   			
    	}
    	else
    	{
    		 
    		allocatePages(vmas);//Allocate pages as per vma size
    		copyELF(vmas); //Copy binary contents
             	
    	}	
    }else if(r.int_number == 128){
        // handled directly
    }
    else
    {
        
        printf("Common : %d\n",r.int_number);
        while(1);
    }
    //return
}


//All faulting addresses should lie withing vma ranges except for stack which can be only SLIGHTLY OFF the end.

struct vma* findVMA(uint64_t addr, int errno)
{
	struct vma* temp = current_pcb->first_vma;
	int stack_cond=0;
	while(temp)
	{
		stack_cond = (temp->start > USTACKLIMIT);
		stack_cond = stack_cond && (temp->start-USTACKLIMIT >= PAGE_SIZE);
		stack_cond = stack_cond && (addr>= ((temp->start)-8));
		if((addr>=temp->start && addr<=temp->end) || stack_cond)
		{
			//Permissions check
			if((errno & 0x2) && (temp->flags_vma & 0x2)) //Page fault by page write
			{		
				return temp;
			}
			if(!(errno & 0x2)) //Page fault by page read
			{	
				return temp;
			}
		}
		temp = temp->next;
	}
	return temp;
}

void allocatePages(struct vma* vmas)
{
	int i=0;
	struct page* newPage=NULL;
	int perms;
	int numPages = ((uint64_t)alignUp((void *)vmas->end) - (uint64_t)alignDown((void *)vmas->start))/PAGE_SIZE;

	if(vmas->flags_vma & 0x2)
		perms = BIT_RW | BIT_PRESENT | BIT_USER;
	else
		perms = BIT_PRESENT | BIT_USER;

	for(i = 0; i<numPages; i++)
	{
		if(!(*pml4Walk(current_pcb->pml4, (uint64_t)(vmas->start+i*PAGE_SIZE))))
		{
			newPage = page_alloc();
			page_insert(current_pcb->pml4,(void *)(vmas->start+i*PAGE_SIZE), newPage, perms);
			//printf("Inserted new page PA:%x at faulting addr: %x\n", getPA(newPage), vmas->start+i*PAGE_SIZE);
    		//printf("walk: %x\n",*pml4Walk(current_pcb->pml4, vmas->start+i*PAGE_SIZE));
		}
		
	}

}


void copyELF(struct vma* vmas)
{
    int i;
    struct ELF_hdr *hdr;
    struct ELF_S_hdr *shdr;
    //struct page* page_temp;
    
    hdr = getFile(current_pcb->fileName);
    //printf("file type is %d", hdr->e_type);
    shdr = (struct ELF_S_hdr *)((uint64_t)getFile(current_pcb->fileName) + (uint64_t)hdr -> e_shoff);
    for (i = 0; i < hdr -> e_shnum; i++)
    {
        if ((shdr -> sh_flags) & SH_ALLOC)
        {
            if((shdr->sh_addr >= vmas->start) && (shdr->sh_addr)<= vmas->end)
            {
                if ((shdr -> sh_type) == 8) 
                    memset((void *)shdr -> sh_addr, 0, (size_t) shdr -> sh_size);
                else
                    memcpy((void *)shdr -> sh_addr, (void *)(getFile(current_pcb->fileName) + shdr -> sh_offset), (size_t) shdr -> sh_size);

            }
           
        }
        shdr++;
    }

}


void copy_on_write_handler(uint64_t faultAddr)
{
	uint64_t * pte=NULL;
    struct page* newPage=NULL;
	struct page* faultPage=NULL;;
	void * buffer = kmalloc(PAGE_SIZE);
	pte = pml4Walk(current_pcb->pml4, faultAddr);
	
	//printf("In COW HANDLER\n");

	if(*pte & BIT_COW)
	{
		
		faultPage = getStrPtrFromPA((void *)(*pte));
		if(faultPage->ref_count==2)//Allocate new writeable page and map it
		{
			//Important: Copy bytes from faulting page to temp buffer
			memcpy(buffer, alignDown((void *)faultAddr), PAGE_SIZE);	
			page_remove(current_pcb->pml4, (void *)faultAddr);
			
			newPage=page_alloc();
			page_insert(current_pcb->pml4, (void *)faultAddr, newPage, BIT_PRESENT | BIT_RW | BIT_USER);	
			//Invalidate tlb?, included in page_remove
			loadcr3(current_pcb->cr3);

			//Copy back bytes from temp buffer
			memcpy(alignDown((void *)faultAddr), buffer, PAGE_SIZE);

			//printf("Inserted new page %x on COW fault on page ref=2\n", getPA(newPage));
			
		}
		else
		{
			//Make page rightable
			*pte = (*pte & ~BIT_COW) | BIT_RW;
			//printf("Made page writeable on COW fault, page ref=1\n");
				
		}		
	}	


}
