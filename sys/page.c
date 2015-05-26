#include <sys/sbunix.h>
#include <sys/page.h>
#include <sys/structs.h>
#include <string.h>
#include <sys/process.h>

extern void * physfree_global;

//Variables Fon Page tables
uint64_t *kernel_pml4;

//Variables for memory initialization
struct page* page_free_list; //Head node in linked list of free pages
struct page* pages; //array of page descriptors

//Aady
struct vma* vma_array; //array of vma structs
struct vma * vma_free_list; //Allocate vma s from here, head node in LL of free vma s 

struct mm_struct* mm_struct_array; //array of mm_structs
struct mm_struct* mm_struct_free_list; //head node in LL of free mm_structs

//17th APR
struct PCB* pcb_array; //array of pcb structs
struct PCB* pcb_free_list; //head node of free pcbs

//Kavana
struct tar_file *tar_file_array;
struct tar_file	*tar_file_free_list;


static void * bump_alloc(int size)
{
	static void * next;
	void * ans;
	if(!next)
	{
		next = alignUp((void *)(KERNBASE + (uint64_t)physfree_global)); //Start allocating memory above the kernel
	}
	else
	{
		next = alignUp(next);
	}

	ans = next;
	next = (void *)((uint64_t)next +size);
	return ans;
	
}

void vm_init()
{
	struct page * temp_page;	
	
	//Allocating memory
	pages = bump_alloc(num_pages * sizeof(struct page)); //About 
	vma_array = bump_alloc(num_vm_areas * sizeof(struct vma)); //about 1 page
	pcb_array = bump_alloc(num_pcbs * sizeof(struct PCB)); //abt 1 Page	
	tar_file_array = bump_alloc(num_tar_files * sizeof(struct tar_file));



	//printf("last addr: %x\n", bump_alloc(0));
	//Initialize page free list
	page_free_init();
	
	//Initialize vma free list
	vma_free_init();	

	//17th APR
	//Initialize pcb free init
	pcb_free_init();

	// Kavana 21st April
	tar_file_free_init();
	

	temp_page=page_alloc();
	kernel_pml4 = (uint64_t *)getVA(temp_page);
	memset((void *)kernel_pml4,0,PAGE_SIZE);


	//15th APR
	//kernel_pml4 = (uint64_t *)bump_alloc(PAGE_SIZE);
	//memset((void *)kernel_pml4,0,PAGE_SIZE);
	

	//Mapping all of physical memory at kernbase
	//NOTE: CURRENTLY NOT ACCOUNTING FOR PERMISSIONS FOR THE RANGE (0x9fc00 to 0x100000) that is inaccessible.
	region_mapping((void *)KERNBASE,0x0, kernel_pml4, num_pages*PAGE_SIZE, BIT_RW | BIT_PRESENT);
	//15th APR
	//region_mapping((void *)KERNBASE, 0x0, kernel_pml4, (size_t)(temp-KERNBASE), BIT_RW);
	
	//15th APR
 	loadcr3((void *)PADDR(kernel_pml4));
	//loadcr3((void *)getPA(temp_page));


}



//CHANGED: AADY: 24th APR
// Align to page size:SHOULD NOT Give a page aligned addr after address 
void * alignUp(void * address)
{
	void * diff = (void *)(PAGE_SIZE - ((uint64_t)address % PAGE_SIZE));
	if((uint64_t)diff == PAGE_SIZE)
		diff = 0;
	return (void *)((uint64_t)address + (uint64_t)diff);
}

void * alignDown(void * address)
{
	void * diff = (void *)((uint64_t)address % PAGE_SIZE);
	return (void *)((uint64_t)address - (uint64_t)diff);
}


//Creating an list of free pages

void page_free_init()
{
	size_t i;
	//printf("Inside pf init: %x\n",physfree_global);
	for(i=0;i<num_pages;i++)
	{
		uint64_t start_addr = (i*PAGE_SIZE);
		uint64_t end_addr = (start_addr + PAGE_SIZE -1);
		if	( (start_addr>=0x9FC00 && start_addr<=0x100000) || 
			(end_addr>=0x9FC00 && end_addr<=0x100000) || (start_addr>=PHYSBASE && start_addr<=(uint64_t)KHEAPEND) || (end_addr>=PHYSBASE && end_addr<=(uint64_t)KHEAPEND))
		{
			//Mark this page as not available
			pages[i].ref_count=1;
		}
		else
		{
			//CHANGED: AADARSH 8th APR
			if(page_free_list==NULL) //Putting first page to free list
			{
				page_free_list = &pages[i];
				page_free_list->next = NULL;
			}	
			else
			{
                        	//Add the page to free list
                        	page_free_list->next = &pages[i];
                        	pages[i].prev = page_free_list; 
                       	 	page_free_list=&pages[i];
                        	pages[i].ref_count=0;

                        	//Setting flags pending 
			}
		}
	}

}//end of page_free_init()

//Allocating a free page from page_free_list
struct page* page_alloc()
{
	//Out of memory
	if(page_free_list==NULL)
		return NULL;

	struct page * temp = page_free_list;
	page_free_list = page_free_list->prev;
	temp->prev=NULL;
	page_free_list->next=NULL;

	uint64_t virt_addr = getVA(temp);	

	//Removed memset: cannot memset here, okay for kernel space not for user
	memset((void *)virt_addr, 0, PAGE_SIZE);	

	return temp;
}

//Given a page, getPA will give the physical addr corresponding to this page
uint64_t getPA(struct page* p)
{
	//Find offset of this page in pages[]
	size_t offset = p-pages; // TO CHECK IF THIS IS CORRECT OR NEED TO DIV BY sizeof(struct page)
	//Multiply offset with PAGE_SIZE to get corresponding phy_addr
	uint64_t phy_addr;
	phy_addr = (uint64_t)(offset*PAGE_SIZE);
	return phy_addr;
}

//Given a page, getVA will give the virtual addr corresponding to this page

uint64_t getVA(struct page* p)
{
	uint64_t virt_addr;
	virt_addr = (uint64_t)(KERNBASE + getPA(p));
	return virt_addr;
}

// KAVANA - added following functions

struct page * getStrPtrFromPA(void * phy_addr)
{
	size_t offset = ((uint64_t)alignDown(phy_addr)) / PAGE_SIZE;
	return &pages[offset];
}

struct page * getStrPtrFromVA(void * virt_addr)
{
	void * phy_addr = (void *)PADDR((uint64_t) virt_addr);
	return getStrPtrFromPA(phy_addr);
}


//PAGE WALK FUNCTIONS
uint64_t *pml4Walk(uint64_t *pml4e, uint64_t v_addr)
{

	uint64_t *pte;
	uint64_t *pdpe;
	struct page *newPage = NULL;
	uint64_t newPhyAddr;
	int pml4off = PML4OFF(v_addr);
	//printf("pml4off=%p\n",pml4off);
	uint64_t pdpe_phy_addr = (uint64_t) *(pml4e + pml4off);
	
	if(!(CHECK_PRESENT(pdpe_phy_addr))){
		// page alloc
		newPage = page_alloc();
		if(newPage == NULL){
			printf("Out of Memory inside pml4Walk \n");
			return NULL;
		}else{
			//14th APR: AADY
			//memset((void *)v_addr,0,PAGE_SIZE);


			//15th APR
			//page_insert(kernel_pml4,(void *) getVA(newPage), newPage, BIT_RW);	
			//region_mapping((void *)getVA(newPage), (void *)getPA(newPage), kernel_pml4, PAGE_SIZE, BIT_RW);		
			
			// inc ref
			newPage->ref_count++;			
			// store the phy addr in the loc with P and RW bit set
			newPhyAddr = getPA(newPage);	//CHANGED: Using getPA() here
			*(pml4e + pml4off) = newPhyAddr | BIT_PRESENT | BIT_RW | BIT_USER;
			pdpe_phy_addr = (uint64_t) *(pml4e + pml4off);  // or pdpe_phy_addr = newPhyAddr
		}	
	}

	//alignDown is needed below to remove the ORed R/W and P bits
	pdpe = (uint64_t *) KADDR(alignDown((void *)pdpe_phy_addr));
	//printf("phy addr of pdpe base= %p\n",pdpe);
	pte = pdpeWalk(pdpe,v_addr);	// Changes might be involved if NULL returned
	if(pte == NULL){
		printf("NULL in pml4Walk\n");
		return NULL;	
	}else
		return pte;
}



uint64_t *pdpeWalk(uint64_t *pdpe,uint64_t v_addr)
{
	uint64_t *pte;
	uint64_t *pde;
	struct page *newPage = NULL;
	uint64_t newPhyAddr;
	int pdpeoff = PDPEOFF(v_addr);
	//printf("pdpeoff=%p\n",pdpeoff);
	uint64_t pde_phy_addr = (uint64_t) *(pdpe + pdpeoff);
	

	if(!(CHECK_PRESENT(pde_phy_addr))){
		// page alloc
		newPage = page_alloc();
		//AADY:14th APR
		//memset((void *)v_addr,0,PAGE_SIZE);	

		//15th APR
                //page_insert(kernel_pml4,(void *)getVA(newPage), newPage, BIT_RW);
		//region_mapping((void *)getVA(newPage), (void *)getPA(newPage), kernel_pml4, PAGE_SIZE, BIT_RW);
		
		if(newPage == NULL){
			printf("Out of Memory");
			return NULL;
		}else{
			// inc ref
			newPage->ref_count++;
			// store the phy addr in the loc with P and RW bit set
			//CHANGED: Using getPA() here
			newPhyAddr = getPA(newPage);
			*(pdpe + pdpeoff) = newPhyAddr | BIT_PRESENT | BIT_RW | BIT_USER;
			pde_phy_addr = (uint64_t) *(pdpe + pdpeoff);  // or pde_phy_addr = newPhyAddr
		}
	}

	pde = (uint64_t *) KADDR(alignDown((void *)pde_phy_addr));
	pte = pdeWalk(pde,v_addr);	// Changes might be involved if NULL returned
	if(pte == NULL){
		printf("NULL in pdpeWalk\n");	
		return NULL;
	}else
		return pte;
}


uint64_t *pdeWalk(uint64_t *pde,uint64_t v_addr)
{
	uint64_t *pte;
	struct page *newPage = NULL;
	uint64_t newPhyAddr=0;
	int pdeoff = PDEOFF(v_addr);
	int pteoff = PTEOFF(v_addr);
	//printf("pdeoff=%p\n",pdeoff);
	//printf("pteoff=%p\n",pteoff);	

	uint64_t pte_phy_addr = (uint64_t) *(pde + pdeoff);
	

	if(!(CHECK_PRESENT(pte_phy_addr))){
		// page alloc
		newPage = page_alloc();
		//AADY:14th APR 
                //memset((void *)v_addr,0,PAGE_SIZE);		


		//15th APR
                //page_insert(kernel_pml4, (void *)getVA(newPage), newPage, BIT_RW);
		//region_mapping((void *)getVA(newPage), (void *)getPA(newPage), kernel_pml4, PAGE_SIZE, BIT_RW);

		if(newPage == NULL){
			printf("Out of Memory\n");
			return NULL;
		}else{
			// inc ref
			newPage->ref_count++;
			// store the phy addr in the loc with P and RW bit set
			newPhyAddr = getPA(newPage);	//CHANGED: Using getPA() here
			*(pde + pdeoff) = newPhyAddr | BIT_PRESENT | BIT_RW | BIT_USER;
			pte_phy_addr = (uint64_t) *(pde + pdeoff);  // or pte_phy_addr = newPhyAddr
		}
	}
	//printf("phys addr of pte base= %p\n",newPhyAddr);
	pte = (uint64_t *) KADDR(alignDown((void *)pte_phy_addr));//alignDown is correct and needed here.
	
	return &pte[pteoff];
}

//Pass virt_addr and phy_addr PAGE ALIGNED
void region_mapping(void * virt_addr, void * phy_addr, uint64_t * pml4e, size_t size, int permissions)
{
	size_t temp = 0;
	uint64_t * ret_val;
 
	while(temp < size)			
	{
		//Note: If PTE for this Virt addr doesnt exist, it'll be handled by walk functions.
		ret_val = pml4Walk(pml4e, (uint64_t)(virt_addr+temp));
		
		
		if(*ret_val)
		{
 			printf("Something already mapped!! \n");
 			printf("Virtual : %x\n",(uint64_t)virt_addr);
 			printf("Phy : %x\n",(uint64_t)phy_addr);
 			printf("ret val : %x\n",(uint64_t)*ret_val);
			//NULL RETURNED, PANIC.
		}
		else
		{
		//Writing page table entry here.
		//NOTE: SETTING PAGE AS PRESENT HERE.
		*ret_val = (uint64_t)(phy_addr+temp) | BIT_PRESENT | permissions; //phy_addr+temp will be aligned if phy_addr is aligned.
		//14th APR
		//INC ref count??	

	
		/*
		//new check: 13th april
		if(((uint64_t)virt_addr+temp)==0xffffffff80200000)
		{
			printf("Ret val addr: %x\n",ret_val);
			printf("phy addr: %x\n",(phy_addr+temp));
			printf("Phy page at ret_val: %x\n",*ret_val);	
		}	
		*/
		}
		temp=temp+PAGE_SIZE;			
	}
}//end of region_mapping

//Gives physical address corresponding to a virtual address
//Checking page table mappings
void * get_mapping(void * virt_addr, uint64_t *pml4e)
{
	//Check tlb_cache first
	//Pending	

	uint64_t * temp = pml4Walk(pml4e,(uint64_t)virt_addr);
	/*
	int offset = PAGEOFF((uint64_t)virt_addr);
	if(!temp)
	{
		printf("pml walks returned null.\n");	
	}	
	return (void *)((uint64_t)alignDown((void *)*temp) + offset);
	*/
	return (void *)(*temp);
}


void testMapping(){
	//Checking mappings
			
	printf("0xffffffff80200000: %p\n",get_mapping((void *)0xffffffff80200000,kernel_pml4));
	printf("0xffffffff8030beef:%p\n",get_mapping((void *)0xffffffff8030beef, kernel_pml4));
/*	printf("0x600000: %p\n",get_mapping((void *)0x600000,kernel_pml4));
	
	printf("0xFFFFFFFF800B8000:%p\n",get_mapping((void *)0xFFFFFFFF800B8000,kernel_pml4));
	printf("0xFFFFFFFF800B8000:%p\n",PADDR(alignDown((void *)0xFFFFFFFF800B8000)));

	*/
	
		
}

// KAVANA - added following functions

//NOTE: Ref count increased here

// page should have been allocated 
void page_insert(uint64_t *pml4e, void * virt_addr, struct page * page, int permissions)
{
	//14th APR: AADY
	virt_addr = alignDown(virt_addr);	

	uint64_t * ret_val = pml4Walk(pml4e, (uint64_t)virt_addr);
	//Store phy addr that pg represents to this address	
	if(*ret_val){
		printf("HERE SOMETHING MAPPED\n");
		printf("Virt addr: %x",virt_addr);
		while(1);
		page_remove(pml4e,virt_addr);
	}
	*ret_val = getPA(page) | BIT_PRESENT | permissions;
	
	//Increase ref count as a ref is being added to this page
	page->ref_count = page->ref_count+1;
}


void page_remove(uint64_t *pml4e,void * virt_addr){
	uint64_t *pte;
	struct page *page;

	pte = pml4Walk(pml4e,(uint64_t)virt_addr);
	if(pte == NULL){
		// Out of memory 
		// Page not present
	}else{
		if(!(*pte)){
			// page not present
		}else{
			page = (struct page *)getStrPtrFromPA(alignDown((void *)*pte));
			page->ref_count--;
			if(page->ref_count == 0)
				page_release(page);
			*pte = 0;
			invalidate_TLB(pml4e,virt_addr);
		}
	}
}


void page_release(struct page *page){
	//printf("Released page:%x\n",getPA(page));
	//Memsetting page contents to zero
	uint64_t * page_va = (uint64_t *)getVA(page);
	memset((void *)page_va, 0, PAGE_SIZE);
	
	page->ref_count = 0;
	page->next = NULL;
	page->prev = page_free_list;
	page_free_list->next = page;
	page_free_list = page;
}

void invalidate_TLB(uint64_t *pml4e,void * virt_addr){
	if(pml4e != NULL)
	 	INVALIDATE_TLB(virt_addr);
}

void loadcr3(void *addr){
	LOAD_CR3(addr);
}

void * readcr3(){
	uint64_t val;
	READ_CR3(val);
	return (void *)val;
}

void * readcr2(){
        uint64_t val;
        READ_CR2(val);
        return (void *)val;
}


//AADARSH : 10th April
void vma_free_init()
{
	int i =0 ;
	for(i=num_vm_areas-1;  i >= 0; i--)
	{
		if(vma_free_list==NULL)
		{
			vma_free_list = &vma_array[i];
			vma_free_list->next=NULL;
			vma_free_list->prev=NULL;
		}
		else
		{
			vma_free_list->prev = &vma_array[i];
			vma_array[i].next = vma_free_list;
			vma_free_list = &vma_array[i];
		} 
	}
}


struct vma* vma_alloc()
{
	struct vma * temp = vma_free_list;
	if(!temp)
	{	
		printf("out of vma s !!!");
		return NULL;	
	}
	vma_free_list =temp->next;
	return temp;

}


//AADARSH : 17th April
void pcb_free_init()
{
        int i =0 ;
        for(i=num_pcbs-1;  i >= 0; i--)
        {
                if(pcb_free_list==NULL)
                {
                        pcb_free_list = &pcb_array[i];
                        pcb_free_list->next=NULL;
                        pcb_free_list->prev=NULL;
                }
		else
		{
                	pcb_free_list->prev = &pcb_array[i];
                	pcb_array[i].next = pcb_free_list;
                	pcb_free_list = &pcb_array[i];
		}
        }
}


struct PCB* pcb_alloc()
{
        struct PCB * temp = pcb_free_list;
        if(!temp)
        {
                printf("out of pcb s !!!");
                return NULL;
        }
        //AADY: 30th APR
        pcb_free_list =temp->next;
        temp->pml4=NULL;
        temp->cr3=NULL;
        temp->first_vma=NULL;
        temp->next=NULL;
        temp->prev=NULL;
        temp->hasRan=0;
        temp->processID=-1; //IMP

        return temp;

}

// Kavana : 21st April 

void tar_file_free_init(){

	int i=0;
	for(i=num_tar_files-1;i>=0; i--)
	{
		if(tar_file_free_list == NULL)
		{
			tar_file_free_list = &tar_file_array[i];
		 	tar_file_free_list->next = NULL;
		}
		else
		{
			tar_file_array[i].next = tar_file_free_list;
			tar_file_free_list = &tar_file_array[i];
		}
	}
}

struct tar_file * tar_file_alloc(){
    struct tar_file * new_file = tar_file_free_list;
    if(new_file==NULL){
            printf("Out of memory for tar files \n");
            return NULL;
    }else{
            tar_file_free_list = new_file->next;
            return new_file;
    }
}




//AADY: 24th APR - INEFFICIENT KMALLOC
void * kmalloc(size_t size)
{
	static uint64_t kernel_heap_end;
	if(!kernel_heap_end)
		kernel_heap_end = KHEAPSTART; 
	uint64_t temp=0;
	if(size>=PAGE_SIZE)
	{
		temp = (uint64_t)alignUp((void *)kernel_heap_end);
		if((temp+size)<KHEAPEND)
		{
	
		kernel_heap_end = temp + size;
		
		return (void *)temp;	
		}
		else
		{
			printf("Out of memory in kernel space.\n");
			return (void *)0;
		}
	}
	
	temp = kernel_heap_end;	
	if(temp+size<KHEAPEND)	
	{
	kernel_heap_end = kernel_heap_end + size;
	memset((void *)temp,0,size);
	return (void *)temp;
		
	}
	else
	{
		printf("Out of memory in kernel space.");
		return (void*)0;
	}
	
}
