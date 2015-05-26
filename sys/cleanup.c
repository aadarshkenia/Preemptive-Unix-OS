#include <sys/sbunix.h>
#include <sys/page.h>
#include <sys/structs.h>
#include <string.h>


//AADY: 30th APR
//Free all pages related to vma s of this process
//Add vmas back to free list
extern struct vma* vma_free_list;
void vma_remove_for_process(struct PCB* process)
{
	struct vma* vma_temp=NULL;
	struct vma* vma_follower=NULL;
	int numpages=0,i=0;
	uint64_t *ret_val=NULL;
	//struct page* temp_page=NULL;


	vma_temp=process->first_vma;

	while(vma_temp)
	{
		vma_follower=vma_temp->next;

		numpages = ((uint64_t)alignUp((void *)vma_temp->end) - (uint64_t)alignDown((void *)vma_temp->start))/PAGE_SIZE;
		for(i=0;i<numpages;i++)
		{
			ret_val=pml4Walk(process->pml4, vma_temp->start + i*PAGE_SIZE);
			if(*ret_val) //If this page is mapped
			{
				
				page_remove(process->pml4, (void *)(vma_temp->start + i*PAGE_SIZE)); //Also reduces ref count					
			}	

		}
		vma_free_list->prev = vma_temp;
		vma_temp->next = vma_free_list;
		vma_free_list=vma_temp;
		vma_temp->start = 0;
		vma_temp->end = 0;
		vma_temp->flags_vma = 0;
		vma_temp->prev=NULL;

		//Go to next vma
		vma_temp = vma_follower;

	}

	process->first_vma = NULL;

//Invalidate tlb in end, either here or in execve handler

}

void delete_page(struct page* del_page)
{
	//uint64_t pte_pa = getPA(del_page);
	del_page->ref_count--;
	if(del_page->ref_count == 0)
	{
		//printf("Released page for %x: \n", pte_pa);
		page_release(del_page);	
	}
}

void delete_process(struct PCB* process)
{
	struct PCB * parent_process=process->parentPCB;
	int i=0,j=0,k=0;
	uint64_t * pdpe_va=NULL;
	uint64_t * pdpe_pa=NULL;
	uint64_t * pde_va=NULL;
	uint64_t * pde_pa=NULL;
	uint64_t * pte_pa=NULL;
	struct page* del_page=NULL;
	


	//Remove all mappings in user space
	vma_remove_for_process(process);

	process->processID=-1;
	process->parentProcessID=-1;
	process->hasRan=-1;
	process->next=NULL;
	process->prev=NULL;
	process->ersp = 0;

	memset((void *)process->fileName,0, strlen(process->fileName));
	process->fileName=NULL;
	process->sleepTimeLeft = 0;
	process->time=0;
	process->waitPID=-2;
	memset((void *)process->cwd,0, strlen(process->cwd));
	process->cwd=NULL;
	
	//NEW 7th MAY:AADY
	
	process->hasExited=1;
	if(process->isFore && parent_process!=NULL && !(parent_process->hasExited))
	{
		process->parentPCB->isFore=1;
		process->isFore=-1;
	}
	
	
	//File descriptors?
	//ASK KAV
	for(i=0;i<MAX_FILE_DESCRIPTORS_PER_PROCESS;i++)
	{
		if(process->fileDescriptors[i])
		{
			process->fileDescriptors[i]->refCount--;
			if(process->fileDescriptors[i]->refCount==0)
				process->fileDescriptors[i]=NULL;
		}
	}


	//Freeing pages occupied by page tables themselves
	//VERY IMP: DONT DELETE LAST ENTRY FOLLOW UP TABLES

	for(i=0;i<NO_OF_TABLE_ENTRIES;i++)
	{
		if(i==PML4OFF(KERNBASE)) //Not deleting kernel page tables
		{
			//printf("Not deleting kernel page tables\n");
			process->pml4[i]=0;
			continue;
		}
		pdpe_pa = (uint64_t *)process->pml4[i];
		
		if(CHECK_PRESENT((uint64_t)pdpe_pa))
		{
			pdpe_va = (uint64_t *)KADDR(alignDown((void *)pdpe_pa));
			for(j=0;j<NO_OF_TABLE_ENTRIES;j++)
			{
				pde_pa = (uint64_t *)*(pdpe_va+j);
				
				if(CHECK_PRESENT((uint64_t)pde_pa))
				{
					pde_va = (uint64_t *)KADDR(alignDown((void *)pde_pa));
					for(k=0;k<NO_OF_TABLE_ENTRIES;k++)
					{
						pte_pa = (uint64_t *)pde_va[k];
						
						if(CHECK_PRESENT((uint64_t)pte_pa))
						{
							//Deletion of pte tables
							del_page = getStrPtrFromPA((void *)pte_pa);
							delete_page(del_page);
							//printf("level 4\n");
						}
						pte_pa=0;
					}

					//Deletion of pde tables
					del_page = getStrPtrFromPA((void *)pde_pa);
					delete_page(del_page);
					//printf("level 3\n");	
				}
				pde_pa=0;
				
			}

			//Deletion of pde tables
			del_page = getStrPtrFromPA((void *)pdpe_pa);
			delete_page(del_page);
			//printf("level 2\n");
		}
		pdpe_pa=0;
		
			
	}//loop traversal complete

	//Deleting pml4 table
	delete_page(getStrPtrFromPA(process->cr3));
	//printf("level 1\n");

	process->pml4 = NULL;
	process->cr3  = NULL;
}//end of delete_process()