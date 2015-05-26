#include <sys/sbunix.h>
#include <sys/gdt.h>
#include <sys/tarfs.h>
#include <sys/page.h>
#include <sys/elf.h>
#include <sys/process.h>
#include <sys/structs.h>
#include <string.h>
//CHANGED

#define INITIAL_STACK_SIZE 4096
int numProcesses = -1;
int global_counter = -1;
char stack[INITIAL_STACK_SIZE];
uint32_t* loader_stack;
extern char kernmem, physbase;
extern uint64_t * kernel_pml4;

void * physfree_global;	


//Keeping track of current process struct
struct PCB *current_pcb = NULL;
struct PCB *pcb_head = NULL;
// struct PCB *sleep_pcb_list = NULL;
struct PCB *sleep_pcb_head = NULL;
struct PCB *wait_pcb_head = NULL;
struct PCB *block_pcb_head = NULL;

char *readBuff;
struct tss_t tss;
void *addressOfBuff = NULL;

extern void init_idt();
extern void init_pic();
extern void init_timer(uint32_t);
extern void init_keyboard();
extern void setupHeirarchyOfTarfs();
extern struct tar_file* lookUpForFile(const char *);
extern struct tar_file *tarfsroot;


void start(uint32_t* modulep, void* physbase, void* physfree)
{
	
	struct smap_t {
		uint64_t base, length;
		uint32_t type;
	}__attribute__((packed)) *smap;
	while(modulep[0] != 0x9001) modulep += modulep[1]+2;

	
	for(smap = (struct smap_t*)(modulep+2); smap < (struct smap_t*)((char*)modulep+modulep[1]+2*4); ++smap) {
		if (smap->type == 1 && smap->length != 0) {
			printf("Available Physical Memory [%x-%x]\n", smap->base, smap->base + smap->length);
		}
	}
	printf("tarfs in [%x:%x]\n", &_binary_tarfs_start, &_binary_tarfs_end);
	

//**************DONT WRITE ANYTHING ABOVE THIS LINE****************
	// kernel starts here
 	
	physfree_global = physfree;
 	//Memory management
	vm_init();
  	//testMapping();	

	init_idt();	
 	init_pic();
 	init_timer(100);
	init_keyboard();
	
	setupHeirarchyOfTarfs();
 	readBuff = kmalloc(PAGE_SIZE); //For read syscall

 	printf("Entering Shell...\n");
	
	create_process("bin/ideal",0);
	//create_process("bin/ps",0);
	//create_process("bin/ls",0);
	 //create_process("bin/kavanachild",0);
	//create_process("bin/bad",1);
	create_process("bin/init",1);	

	//ASM to set up tss    
    __asm__ __volatile__("movq $0x28,%%rax; \
                ltr %%ax;"
                :
                :
                : "rax");

 	
    __asm__ __volatile__("sti");

    // Kavana testing for fs sys calls
    
	//  struct tar_file *node = lookUpForFile("kavtest/test/");
	//  if(node == NULL){
	//  	printf("no node\n");
	//  }else{
	//  	printf("node present\n");
	//  	printf("node content:%s\n",node->start);
	// 	printf("node size:%d\n",node->fileSize);
	// 	printf("node perm:%o\n",node->filePerm);
	// }
	 
	
	
// 	char *str = (char *)kmalloc(100);
// 	memset(str,0,100);
// 	strncpy(str,(char *)node->start+54,15);
// 	printf("after copy :%s\n",str);
// 	printf("bytes read:%d\n",strlen(str));
	
//  	c=a/b;
//     printf("%d\n",c);

   	
}


void boot(void)
{
	// note: function changes rsp, local stack variables can't be practically used
	//register char *s; // *v;
	__asm__(
		
		"movq %%rsp, %0;"
		"movq %1, %%rsp;"
		:"=g"(loader_stack)
		:"r"(&stack[INITIAL_STACK_SIZE])
	);

	reload_gdt();
	setup_tss();

	start(
		(uint32_t*)((char*)(uint64_t)loader_stack[3] + (uint64_t)&kernmem - (uint64_t)&physbase),
		&physbase,
		(void*)(uint64_t)loader_stack[4]
	);
	//s = "!!!!! start() returned !!!!!";
	//printf("%s\n",s);
	//for(v = (char*)0xb8000; *s; ++s, v += 2) *v = *s;
	while(1);
}
