#ifndef _PAGE_H
#define _PAGE_H

#include <sys/defs.h>
#include <stdlib.h>

#define PAGE_SIZE 4096
#define PAGE_SHIFT 12
#define NO_OF_TABLE_ENTRIES 512
#define num_pages 32766 //total physical pages, 0x7ffe000 / 0x1000
#define num_vm_areas 100
//size_t num_mm_structs = 10; //Not needed for now
#define num_pcbs 1000 
#define num_tar_files 100 // Kavana


//KERNBASE - where kernel space starts i.e, region kernel can access
//KERNEL_START - start of where kernel resides
#define KERNBASE 0xFFFFFFFF80000000
#define PHYSBASE 0x200000
#define PHYSFREE 0x2A2000
#define KERNEL_START  KERNBASE+PHYSBASE
#define KERNEL_END KERNBASE+PHYSFREE

// Addresses should be page aligned use alignDown before calling KADDR and PADDR
#define KADDR(PHYADDR) (KERNBASE + (uint64_t)PHYADDR) 
#define PADDR(KVADDR) ((uint64_t)KVADDR  - KERNBASE) 
//AADY 10 APR


//Changed: AADY: 25th APR

#define USTACKTOP   0xFFFFFF7F7FFFE000      	
#define USTACKLIMIT 0XFFFFFF7F7FEFE000   	 
#define UHEAPSTART  0XFFFFFF7F7FDFC000
#define UHEAPLIMIT  0XFFFFFF7F7FEFC000        

#define UXSTACKTOP 0xFFFFFFFF80500000
#define KHEAPSTART 0xFFFFFFFF80502000 //AADY: 24TH APR
#define KHEAPEND   0xFFFFFFFF80600000		

// Get offset of pml4 , pdpe , pde and pte from va
#define PML4OFF(VADDR) ((VADDR >> 39) & 0x1FF)
#define PDPEOFF(VADDR) ((VADDR >> 30) & 0x1FF)
#define PDEOFF(VADDR) ((VADDR >> 21) & 0x1FF)
#define PTEOFF(VADDR) ((VADDR >> 12) & 0x1FF)
#define PAGEOFF(VADDR) (VADDR & 0xFFF)

// first 3 bits in page table entries
#define BIT_PRESENT 1	// Present bit
#define BIT_RW 2		// Read Write bit
#define BIT_USER 4		// User - Supervisor bit
//AADY:24 APR
#define BIT_COW 256		//Copy on write bit(9th bit)- reserved

#define CHECK_PRESENT(phy_addr) (phy_addr & BIT_PRESENT)
#define CHECK_RW(phy_addr)  (phy_addr & BIT_RW)
#define CHECK_USER(phy_addr)  (phy_addr & BIT_USER)

#define LOAD_CR3(val) __asm__ __volatile__ ("movq %0, %%cr3;" :: "r"(val));
#define READ_CR3(val) __asm__ __volatile__ ("movq %%cr3, %0;" : "=r"(val));
#define READ_CR2(val) __asm__ __volatile__ ("movq %%cr2, %0;" : "=r"(val));

#define INVALIDATE_TLB(val) __asm__ __volatile("invlpg (%0)" : : "r" (val) : "memory");


//Initializing Kernel data structures

void page_free_init();
void vma_free_init();
void pcb_free_init();
void tar_file_free_init(); // Kavana

//Allocating vma s and pcb s
struct vma* vma_alloc();
struct PCB* pcb_alloc();
struct tar_file* tar_file_alloc(); // Kavana


//Setting up kernel address space

void vm_init();
// static void * bump_alloc(int size);
void region_mapping(void * virt_addr, void * phy_addr, uint64_t * pml4e, size_t size, int permissions);
void * get_mapping(void * virt_addr, uint64_t *pml4e);
void testMapping();
void * kmalloc(size_t size); //AADY: 24 APR

//Paging related

struct page* page_alloc();
uint64_t *pml4Walk(uint64_t *pml4, uint64_t v_addr);
uint64_t *pdpeWalk(uint64_t *pdpe,uint64_t v_addr);
uint64_t *pdeWalk(uint64_t *pde,uint64_t v_addr);
void page_insert(uint64_t *pml4e, void * virt_addr, struct page * page, int permissions);
void page_remove(uint64_t *pml4e,void * virt_addr);	// KAVANA - added following functions
void page_release(struct page *page);
struct page* getStrPtrFromPA(void * phy_addr);//PA to page
struct page* getStrPtrFromVA(void * virt_addr);//VA to page


//Utility

void * alignUp(void * address);
void * alignDown(void * address);
uint64_t getVA(struct page* p);//page to VA
uint64_t getPA(struct page* p);//page to PA

// KAV
void loadcr3(void *addr);
void * readcr3();
void * readcr2();
void invalidate_TLB(uint64_t *pml4e,void * virt_addr);


#endif
