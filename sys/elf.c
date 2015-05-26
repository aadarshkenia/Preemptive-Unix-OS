#include <sys/elf.h>
#include <sys/defs.h>
#include <sys/tarfs.h>
#include <sys/sbunix.h>
#include <sys/page.h>
#include <string.h>
#include <sys/process.h>


//AADY: 17th APR
int check_vma(uint64_t addr, size_t size, struct PCB* process, int flags);
void insert_vma(uint64_t addr, size_t size, struct PCB* process, int flags);	
void set_vma(struct PCB* process, struct vma* curr_vma, struct vma* prev_vma, struct vma* next_vma, uint64_t start_addr, uint64_t end_addr, int vma_flags);
int get_vma_flags(int sh_flags); //Gives vma flags for corresponding sh_flags

//Magic number check
int checkELF(char *fileName) {
	struct ELF_hdr *hdr;
	hdr = getFile(fileName);
	if(hdr == NULL) return 0;
	char *testIfELF = (char *)kmalloc(5);
	memcpy(testIfELF, hdr, 4);
	if (strcmp(testIfELF, "\177ELF")) return 0;
	return 1;
}

uint64_t getELF(char *fileName, struct PCB* process) {
	int i;
	struct ELF_hdr *hdr;
	struct ELF_S_hdr *shdr;

	hdr = getFile(fileName);
	
	shdr = (struct ELF_S_hdr *)((uint64_t)hdr + (uint64_t)hdr -> e_shoff);
	for (i = 0; i < hdr -> e_shnum; i++) {
		if ((shdr -> sh_flags) & SH_ALLOC) if(!check_vma(shdr->sh_addr, (size_t)shdr->sh_size, process, shdr->sh_flags)) insert_vma(shdr->sh_addr,(size_t)shdr->sh_size, process, shdr->sh_flags);
		shdr++;
	}
	return hdr -> e_entry;
}


//Checks if this addr is present in any vma range and whether PERMISSIONS match, if yes, returns 1, else 0.

//Flags defined in elf.h

int check_vma(uint64_t addr, size_t size, struct PCB* process, int flags)
{
	struct vma* temp_vma = process->first_vma;
	if(!temp_vma)
		return 0;

	while(temp_vma!=NULL)
	{
		if(addr>=temp_vma->start && addr<=temp_vma->end) 
		{
			if((addr+size)>temp_vma->end)
			{
				//update vma size if end is larger
				temp_vma->end = (uint64_t)alignUp((void *)(addr+size));
			}


			/*
			if((flags==(SH_EXEC | SH_ALLOC)) && (temp_vma->flags_vma == (VM_EXEC | VM_READ)))
				return 1;
			if((flags==(SH_WRITE | SH_ALLOC)) && (temp_vma->flags_vma == (VM_WRITE | VM_READ)))
				return 1;
			if(flags==SH_ALLOC && temp_vma->flags_vma == VM_READ)
				return 1;
			*/
			//New: 27th APR
			return 1;

		}
		temp_vma=temp_vma->next;
	}
	return 0;
}

//Creates a new vma and inserts it into its correct position
//Need to align down addr here
//Handling 2 cases

void insert_vma(uint64_t addr,size_t size, struct PCB* process, int flags)
{
	struct vma* temp_vma = process->first_vma;
	struct vma* temp = vma_alloc();
	int vma_flags = get_vma_flags(flags);

	uint64_t start_addr = (uint64_t)alignDown((void *)addr);
	uint64_t end_addr = (uint64_t)alignUp((void *)(addr+size));

	if(temp_vma==NULL)
	{
		set_vma(process, temp, NULL, NULL, start_addr, end_addr, vma_flags);
		return;
	}
	
	while(temp_vma->next!=NULL)
	{
		if(addr>=temp_vma->start && addr<=temp_vma->end)
		{
			set_vma(process, temp,temp_vma, temp_vma->next, start_addr, end_addr, vma_flags);
			return;
		}
		temp_vma=temp_vma->next;
	}
	//Insert at end
	set_vma(process, temp, temp_vma, NULL, start_addr, end_addr, vma_flags);
}


void set_vma(struct PCB* process, struct vma* curr_vma, struct vma* prev_vma, struct vma* next_vma, uint64_t start_addr, uint64_t end_addr, int vma_flags)
{
	//Adding first vma
	if(!prev_vma && !next_vma)
	{
		process->first_vma = curr_vma;
	}
	curr_vma->start = start_addr;
	curr_vma->end = end_addr;
	curr_vma->next = next_vma;
	curr_vma->prev = prev_vma;
	if(next_vma)
		next_vma->prev = curr_vma;
	if(prev_vma)
		prev_vma->next = curr_vma;
	
	curr_vma->flags_vma = vma_flags;
}


//Get vma flags for corresponding section header flags
int get_vma_flags(int shf)
{
	if(shf==0x6)//Execute perm
		return 0x5;
	if(shf==0x3)//Write perm
		return 0x3;
	if(shf==0x2)//Read perm
		return 0x1;//Changed
	if(shf==0x7)//Write+exec
		return 0x7;
	return 0x2; //Default read perm
}
