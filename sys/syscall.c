#include <sys/syscall.h>
#include <sys/defs.h>
#include <sys/sbunix.h>
#include <string.h>
#include <stdlib.h>
#include <sys/gdt.h>
#include <sys/page.h>
#include <sys/process.h>
#include <sys/structs.h>
#include <sys/cleanup.h>
#include <sys/elf.h>
//CHANGED

#define MAX_LENGTH_FILENAME 128		// KAVANA
#define PIPE_SIZE 5*PAGE_SIZE
extern struct PCB *current_pcb;
extern struct tar_file *tarfsroot;
extern struct tar_file* lookUpForFile(const char *);
extern uint64_t convertOctalToDecimal(uint64_t );
extern void parseFileName(const char *string,char **args,int *count);
extern struct PCB *sleep_pcb_head;
extern struct PCB *wait_pcb_head;
extern struct PCB *block_pcb_head;
extern struct PCB *pcb_head;
extern struct tss_t tss;
extern int numProcesses;
extern struct vma * vma_free_list;
extern char *readBuff;
extern void *addressOfBuff;
extern uint64_t *kernel_pml4;


void PSisCalled(struct PCB *head) {
	struct PCB *temp = head;
	int s = 0;
	while (temp) {
		s = temp -> time;
		if (temp -> hasRan) printf("%d               %d               %s\n", temp -> processID, s, temp -> fileName);
		temp = temp -> next;
	}
}

void SchNext() {
	int a = 0;
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

void sys_yield(struct regs *r) { // Aaswad April 26th
	struct PCB *temp = current_pcb -> next;
	if(current_pcb -> hasRan) {
		//Change this : IMP
		//current_pcb -> reg = *r; // done
		current_pcb->reg.ex_rsp = (uint64_t)r;
	}
	if (temp != NULL) temp = pcb_head;
	current_pcb = temp;
	SchNext();
}

void sleepPlease(struct regs *r, unsigned int timeSec) { // Aaswad April 26th
	struct PCB *temp = sleep_pcb_head;
	struct PCB *temp1 = current_pcb;
	struct PCB *temp2 = current_pcb -> next;
	if (current_pcb == pcb_head) {
		if (current_pcb -> next != NULL) pcb_head = current_pcb -> next;
	}
	if (current_pcb -> next == NULL) temp2 = pcb_head;
	while (temp != NULL) {
		if (!temp -> next) break;
		temp = temp -> next;
	}
	if (temp1 -> next != NULL) (temp1 -> next) -> prev = temp1 -> prev;
	if (temp1 -> prev != NULL) (temp1 -> prev) -> next = temp1 -> next;
	if (sleep_pcb_head != NULL) {
		temp -> next = temp1;
		temp1 -> prev = temp;
		temp1 -> next = NULL;
		temp1 -> sleepTimeLeft = timeSec;
	} else {
		sleep_pcb_head = temp1;
		sleep_pcb_head -> next = NULL;
		sleep_pcb_head -> prev = NULL;
		sleep_pcb_head -> sleepTimeLeft = timeSec;
	}
	if(current_pcb -> hasRan) {
		//Change this : IMP
		//current_pcb -> reg = *r; // done
		current_pcb->reg.ex_rsp = (uint64_t)r;
	}
	current_pcb = temp2;
	SchNext();
}

int isPresent(struct PCB *head, int PID) {
	struct PCB *temp = head;
	while (temp) {
		if (temp -> processID == PID) return 1;
		temp = temp -> next;
	}
	return 0;
}

int isChildPresent(struct PCB *head) {
	int PID = current_pcb -> processID;
	struct PCB *temp = head;
	while (temp) {
		if (temp -> parentProcessID == PID) return 1;
		temp = temp -> next;
	}
	return 0;
}

void waitPlease(struct regs *r, int wPID) { // Aaswad May 1st
	if(current_pcb -> processID != wPID) { //CHANGED: 7th MAY
		struct PCB *temp = wait_pcb_head;
		struct PCB *temp1 = current_pcb;
		struct PCB *temp2 = current_pcb -> next;
		if (wPID == -1){
			if (!(isChildPresent(wait_pcb_head) || isChildPresent(sleep_pcb_head) || isChildPresent(pcb_head) || isChildPresent(block_pcb_head))) return;
		} else {
			if (!(isPresent(wait_pcb_head, wPID) || isPresent(sleep_pcb_head, wPID) || isPresent(pcb_head, wPID) || isPresent(block_pcb_head, wPID))) return;
		}
		if (current_pcb == pcb_head) if (current_pcb -> next != NULL) pcb_head = current_pcb -> next;
		if (current_pcb -> next == NULL) temp2 = pcb_head;
		while (temp != NULL) {
			if (!temp -> next) break;
			temp = temp -> next;
		}
		if (temp1 -> next != NULL) (temp1 -> next) -> prev = temp1 -> prev;
		if (temp1 -> prev != NULL) (temp1 -> prev) -> next = temp1 -> next;
		if (wait_pcb_head != NULL) {
			temp -> next = temp1;
			temp1 -> prev = temp;
			temp1 -> next = NULL;
			temp1 -> waitPID = wPID;
		} else {
			wait_pcb_head = temp1;
			wait_pcb_head -> next = NULL;
			wait_pcb_head->prev = NULL;
			wait_pcb_head -> waitPID = wPID;
		}
		if(current_pcb -> hasRan) current_pcb->reg.ex_rsp = (uint64_t)r;
		current_pcb = temp2;
		SchNext();
	}
}

void deleteNode(struct PCB *head, struct PCB *node) {
	struct PCB *wtemp = head;
	if (node == current_pcb)
	{
		if (current_pcb -> next)
		{
			current_pcb = current_pcb -> next;
		} else {
			current_pcb = pcb_head;
		}
	}
	if (wtemp == node)
	{
		if(wtemp -> next) (wtemp -> next) -> prev = wtemp -> prev;
		head = wtemp->next;
	}
	else{
		while (wtemp) {
			if (wtemp == node) {
				if(wtemp -> prev) (wtemp -> prev) -> next = wtemp -> next;
				if(wtemp -> next) (wtemp -> next) -> prev = wtemp -> prev;
				wtemp = NULL;
				return;
			}
			wtemp = wtemp -> next;
		}
	}
}

void sys_exit(struct PCB *temp) {
	//CODE FOR WAITPID
	struct PCB *temp1 = pcb_head;
	while (temp1 != NULL) { 
		temp1 = temp1 -> next;
	}
	temp1 = pcb_head;
	struct PCB *temp2;
	struct PCB *wtemp = wait_pcb_head;
	int wpid = temp -> processID;
	if (wtemp != NULL) {
		while ((temp1 -> next) != NULL) temp1 = temp1 -> next;
		while (wtemp != NULL) {
			temp2 = wtemp -> next;
			if ((wtemp -> waitPID == wpid) || ((wtemp -> waitPID == -1) && (wtemp -> processID == temp -> parentProcessID))) {
				if(!(wtemp -> next) && !(wtemp -> prev)) wait_pcb_head = NULL;
				if(wtemp -> prev) (wtemp -> prev) -> next = wtemp -> next;
				if(wtemp -> next) (wtemp -> next) -> prev = wtemp -> prev;
				temp1 -> next = wtemp;
				wtemp -> prev = temp1;
				// wtemp -> sleepTimeLeft = 0;
				wtemp -> waitPID = -2;
				wtemp -> next = NULL;
				temp1 = wtemp;
			}
			wtemp = temp2;
		}
	}
	//Aaswad Ends
	/*if (current_pcb -> prev) (current_pcb -> prev) -> next = current_pcb -> next; else pcb_head = current_pcb -> next;
	if (current_pcb -> next) {
		(current_pcb -> next) -> prev = current_pcb -> prev;
		current_pcb = current_pcb -> next;
	} else current_pcb = pcb_head;*/

	//VERY IMP
	loadcr3((void *)PADDR(kernel_pml4)); //THE temp's pml4 will be deleted.

	//PROCESS DELETION : Freeing resources
	deleteNode(pcb_head, temp);
	deleteNode(sleep_pcb_head, temp);
	deleteNode(wait_pcb_head, temp);
	deleteNode(block_pcb_head, temp);
	delete_process(temp);
	temp = NULL;
	SchNext();
}

int KillisCalled(struct PCB *head, uint64_t killPID) {
	struct PCB *temp = head;
	while (temp) {
		if (temp -> processID == killPID) {
			sys_exit(temp);
			return 1;
		}
		temp = temp -> next;
	}
	return 0;
}

int sys_open(const char *buf, int flags){
	struct tar_file *filePtr = lookUpForFile((const char *)buf);
	int i;

	if(filePtr == NULL){		
		printf("No such file\n");
		return -1;
	}else{
		if((filePtr->fileType == REGTYPE) && filePtr->filePerm == convertOctalToDecimal(755)){
			printf("Cannot open an executable file\n");
			return -1;
		}
		if(flags == O_WRONLY){
			printf("Writing files in tarfs is not supported\n");
			return -1;
		}
		if(flags == O_CREAT){
			printf("File already exists\n");
			return -1;
		}
		if(flags == O_DIRECTORY &&  !(filePtr->fileType == DIRTYPE)){
			printf("Path specified is not a directory\n");
			return -1;
		}
		for(i=3;i<=MAX_FILE_DESCRIPTORS_PER_PROCESS;i++){
			if(current_pcb->fileDescriptors[i] == NULL){
				break;
			}
		}
		if(i>MAX_FILE_DESCRIPTORS_PER_PROCESS){
			printf("Exceed max file descriptors\n");
			return -1;
		}
 		struct file_descriptor *fd = (struct file_descriptor *)kmalloc(sizeof(struct file_descriptor));
 		fd->fdType = TARFILE;
 		fd->pipe_ptr = NULL;
		fd->offset = 0;
		fd->perm = flags;
		fd->file_ptr = filePtr;
		fd->refCount = 1;
		current_pcb->fileDescriptors[i] = fd;
		return i;	
	}
}

size_t strlenRead(char *string) {
	size_t count = 0;
	while (*string) {
		if (*string == '\n') break;
		string++;
		count++;
	}
	return count;
}
ssize_t sys_read(int fd, void *buf, size_t count, struct regs *r){
	if(fd==0){		//STDIN
		struct file_descriptor *fileDesc = current_pcb->fileDescriptors[fd];
		if(fileDesc->fdType == STDFILETYPE){
			// printf("std in read\n");
			ssize_t len;
			ssize_t fl;
			char *temp = readBuff;
			char *temp1 = NULL;
			if (current_pcb -> isFore) {
				if (*readBuff) {
					len = strlenRead(readBuff);
					fl = strlen(readBuff);
					temp1 = (char *)kmalloc(fl);
					strncpy((char *)buf, (char *)readBuff, len);
					strncpy((char *)temp1, (char *)readBuff + len + 1, fl - len - 1);
					memset(temp, 0, fl);
					memcpy(temp, temp1, fl - len - 1);
					
					//temp[len + 2] = '\0';
					readBuff = temp;
					return len;
				} else {
					if(current_pcb -> hasRan) current_pcb -> reg.ex_rsp = (uint64_t)r;
					if(!block_pcb_head)	
						block_pcb_head = current_pcb;
					else	
						printf("Someone already in read block queue.\n");
					addressOfBuff = buf;
					if (current_pcb -> prev)
						(current_pcb -> prev) -> next = current_pcb -> next; 
					else 
						pcb_head = current_pcb -> next;

					if (current_pcb -> next) 
					{
						(current_pcb -> next) -> prev = current_pcb -> prev;
						current_pcb = current_pcb -> next;
					} 
					else 
						current_pcb = pcb_head;


					//Aady: 5th MAY
					block_pcb_head->next = NULL;
					block_pcb_head->prev = NULL;


					SchNext();
				}
			} else 
				return -1;
		}else if(fileDesc->fdType == PIPEREAD){
			// printf("pipe read\n");
			struct pipe_node *pipeNode = fileDesc->pipe_ptr;
			if(pipeNode == NULL){
				printf("[sys_read]Something is wrong, no pipe ptr\n");
				return -1;
			}
			int w = pipeNode->w;
			uint64_t sz = pipeNode->size;
			int r = pipeNode->r;
			// uint64_t off = fileDesc->offset;

			if(r==0 && w==-1){  // same as (r-w)==1 && w==-1
				printf("Buffer is empty\n");
				if(pipeNode->isWriteClosed){
					printf("Everything is read\n");
					return 0;
				}else{
				//WAIT
				// return 0;
				}
			}
			if(((r-w)==1 && w!=-1)|| (w==(sz-1) && r==0)){	// Full buffer
				if(count>=sz){
					memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+r),sz);
					pipeNode->r = (pipeNode->r + sz)%sz;
					// buffer should be empty
					pipeNode->r = 0;
					pipeNode->w = -1;
					return sz;
				}else{
					memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+(r)),count);
					pipeNode->r = (pipeNode->r + count)%sz;
					return count;
				}
			}
			if(r<=w){
				if(count >= (w-r+1)){
					memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+r),(w-r+1));
					pipeNode->r = pipeNode->r + (w-r+1);
					// buffer should be empty
					pipeNode->r = 0;
					pipeNode->w = -1;
					return (w-r+1);
				}else{
					memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+(r)),count);
					pipeNode->r = pipeNode->r + count;
					return count;
				}
			}else{ 
				if(count>=(sz-r+w+1)){
					memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+r),(sz-r));
					r=0;
					memcpy((void *)(buf+(sz-r)),(pipeNode->pipe_buffer+r),(w+1));
					pipeNode->r = (pipeNode->r + (sz-r+w+1))%sz;
					// buffer should be empty
					pipeNode->r = 0;
					pipeNode->w = -1;
					return (sz-r+w+1);
				}else{
					if(count<=(sz-r)){
						memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+(r)),count);
						pipeNode->r = (pipeNode->r + count)%sz;
						return count;
					}else{
						memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+(r)),(sz-r));
						r=0;
						memcpy((void *)(buf+(sz-r)),(void *)(pipeNode->pipe_buffer+(r)),count-(sz-r));
						pipeNode->r = (pipeNode->r + count)%sz;
						return count;
					}
				}
			}
		}else{
			printf("[sys_read] something wrong, no pointer with fd = 1\n");
			return -1;
		}
	}else if (fd==1){
		printf("STDOUT is not a valid filedescriptor for read\n");
		return -1;
	}else if(fd==2){

	}
	struct file_descriptor *fileDesc = current_pcb->fileDescriptors[fd];
	uint64_t bytesRead;	
	if(fileDesc == NULL){
		printf("Not a valid file descriptor\n");
		return -1;
	}
	if(fileDesc->fdType == TARFILE){
		if(fileDesc->perm == O_WRONLY){
			printf("Permission denied\n");
			return -1;
		}
		struct tar_file *filePtr = fileDesc->file_ptr;
		uint64_t curPos = fileDesc->offset;
		if(filePtr == NULL){
			printf("Something is wrong! No appropriate file to sys_read\n");
			return -1;
		}	
		if(filePtr->fileType == DIRTYPE){
			printf("Not valid operation on directory type file\n");
			return -1;
		}		
		if(curPos >= filePtr->fileSize)
			return 0;
	
		strncpy((char *)buf,(char *)filePtr->start+curPos,count);	
		bytesRead = strlen((char *)buf);
		fileDesc->offset += bytesRead;

		return bytesRead;
	}else if(fileDesc->fdType == PIPEWRITE){
		printf("Invalid file descriptor: expected pipe read not pipe write\n");
		return -1;
	}else if(fileDesc->fdType == PIPEREAD){
		struct pipe_node *pipeNode = fileDesc->pipe_ptr;
		if(pipeNode == NULL){
			printf("[sys_read]Something is wrong, no pipe ptr\n");
			return -1;
		}
		int w = pipeNode->w;
		uint64_t sz = pipeNode->size;
		int r = pipeNode->r;
		// uint64_t off = fileDesc->offset;

		if(r==0 && w==-1){  // same as (r-w)==1 && w==-1
			printf("Buffer is empty\n");
			if(pipeNode->isWriteClosed){
				printf("Everything is read\n");
				return 0;
			}else{
			//WAIT
			// return 0;
			}
		}
		if(((r-w)==1 && w!=-1)|| (w==(sz-1) && r==0)){	// Full buffer
			if(count>=sz){
				memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+r),sz);
				pipeNode->r = (pipeNode->r + sz)%sz;
				// buffer should be empty
				pipeNode->r = 0;
				pipeNode->w = -1;
				return sz;
			}else{
				memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+(r)),count);
				pipeNode->r = (pipeNode->r + count)%sz;
				return count;
			}
		}
		if(r<=w){
			if(count >= (w-r+1)){
				memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+r),(w-r+1));
				pipeNode->r = pipeNode->r + (w-r+1);
				// buffer should be empty
				pipeNode->r = 0;
				pipeNode->w = -1;
				return (w-r+1);
			}else{
				memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+(r)),count);
				pipeNode->r = pipeNode->r + count;
				return count;
			}
		}else{ 
			if(count>=(sz-r+w+1)){
				memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+r),(sz-r));
				r=0;
				memcpy((void *)(buf+(sz-r)),(pipeNode->pipe_buffer+r),(w+1));
				pipeNode->r = (pipeNode->r + (sz-r+w+1))%sz;
				// buffer should be empty
				pipeNode->r = 0;
				pipeNode->w = -1;
				return (sz-r+w+1);
			}else{
				if(count<=(sz-r)){
					memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+(r)),count);
					pipeNode->r = (pipeNode->r + count)%sz;
					return count;
				}else{
					memcpy((void *)buf,(void *)(pipeNode->pipe_buffer+(r)),(sz-r));
					r=0;
					memcpy((void *)(buf+(sz-r)),(void *)(pipeNode->pipe_buffer+(r)),count-(sz-r));
					pipeNode->r = (pipeNode->r + count)%sz;
					return count;
				}
			}
		}
	}else{
		printf("[sys_read]Something is wrong, invalid fd\n");
		return -1;
	}
}

ssize_t sys_write(int fd, const void *buf, size_t size, struct regs *r)
{

	ssize_t ret = 0;
	if(fd == 1){				
		struct file_descriptor *fileDesc = current_pcb->fileDescriptors[fd]; //STDOUT
		if(fileDesc->fdType == STDFILETYPE){
			// printf("std out write\n");
			ret = strlen(buf);
			printf("%s", buf);
			return ret;
		}else if(fileDesc->fdType == PIPEWRITE){
			// printf("pipe write\n");
			struct pipe_node *pipeNode = fileDesc->pipe_ptr;
			if(pipeNode == NULL){
				printf("[sys_write]Something is wrong, no pipe ptr\n");
				return -1;
			}
			int w = pipeNode->w;
			uint64_t sz = pipeNode->size;
			int r = pipeNode->r;
			uint64_t off = fileDesc->offset;
			// printf("size:%d\n",size);
			// printf("off:%d\n",off);

			if(((r-w)==1 && w!=-1) || (w==(sz-1) && r==0)){
				printf("Buffer is full\n");
				// WAIT
				// return 0;
			}
			if(r==0 && w==-1){	// Empty buffer  // same as (r-w)==1 && w==-1
				if((size-off)<=sz){
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),(size-off));
					pipeNode->w = (pipeNode->w + (size-off))%sz;
					fileDesc->offset = fileDesc->offset + (size-off);
					off = fileDesc->offset;
					// printf("w:%d,size:%d\n",pipeNode->w,size);
					// printf("1\n");
					fileDesc->offset = 0;
					return size;
				}else{
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),sz);
					pipeNode->w = (pipeNode->w + sz)%sz;
					fileDesc->offset = fileDesc->offset + sz;
					off = fileDesc->offset;
					// WAIT
					// return sz; 
				}
			}
			if(w<r){
				if((size-off)<(r-w)){
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),(size-off));
					pipeNode->w = pipeNode->w + (size-off);
					fileDesc->offset = fileDesc->offset + (size-off);
					off = fileDesc->offset;
					// printf("2\n");
					fileDesc->offset = 0;
					return size;
				}else{
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),((r-w)-1));
					pipeNode->w = pipeNode->w + (pipeNode->r - pipeNode->w) - 1;	// At this point if w == r - 1 i.e., (r-w)==1 then buffer is full
					fileDesc->offset = fileDesc->offset + ((r-w)-1);
					off = fileDesc->offset;
					// WAIT
					// return ((r-w)-1);
				}
			}else{ 
				if((size-off)<(sz-w+r)){
					if((size-off)<(sz-w)){
						memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),(size-off));
						pipeNode->w = (pipeNode->w + (size-off))%sz;
						fileDesc->offset = fileDesc->offset + (size-off);
						off = fileDesc->offset;
						// printf("3\n");
						fileDesc->offset = 0;
						return size;
					}else{
						memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),sz-w-1);
						// fileDesc->offset = fileDesc->offset + (sz-w-1);
						off = off + (sz-w-1);
						w = -1;
						memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),((size-fileDesc->offset)-(sz-w-1)));
						// fileDesc->offset = fileDesc->offset + ((size-off)-(sz-w-1));
						off = off + ((size-off)-(sz-w-1));
						pipeNode->w = (pipeNode->w + (size-(fileDesc->offset)))%sz;
						fileDesc->offset = off;
						// printf("4\n");
						fileDesc->offset = 0;
						return size;
					}
				}else{
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),sz-w-1);
					// fileDesc->offset = fileDesc->offset + (sz-w-1);
					off = off + (sz-w-1);
					w=-1;
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),r);
					// fileDesc->offset = fileDesc->offset + r;
					off = off + r;
					pipeNode->w = (pipeNode->w + (sz-w-1+r))%sz;
					fileDesc->offset = off;
					//WAIT
					// return (sz-w-1+r);
				}
			}
			printf("[sys_write] Control should never reach here\n");
			return -1;
		}else{
			printf("[sys_write]something is wrong, invalid fdtype");
		}
	}
	if(fd == 2){				// Writing errors to console
		ret = strlen(buf);
		printf("%s", buf);
		return ret;
	}
	if(fd == 0){
		printf("STDIN is not a valid filedescriptor for write\n");
		return -1;
	}
	struct file_descriptor *fileDesc = current_pcb->fileDescriptors[fd];
	if(fileDesc == NULL){
		printf("Not a valid file descriptor\n");
		return -1;
	}
	if(fileDesc->fdType == TARFILE){
		if(fileDesc->perm == O_RDONLY){
			printf("Permission denied\n");
			return -1;
		}else if(fileDesc->perm == O_WRONLY || fileDesc->perm == O_RDWR || fileDesc->perm == O_DIRECTORY){
			printf("Write is not supported for the current file system\n");
			return -1;
		}else{
			printf("Write is not supported for the current file system [sys_write]\n");
			return -1;
		}
	}else if(fileDesc->fdType == PIPEREAD){
		printf("Invalid file descriptor: expected pipe write not pipe read\n");
		return -1;
	}else if(fileDesc->fdType == PIPEWRITE){

		struct pipe_node *pipeNode = fileDesc->pipe_ptr;
		if(pipeNode == NULL){
			printf("[sys_write]Something is wrong, no pipe ptr\n");
			return -1;
		}
		int w = pipeNode->w;
		uint64_t sz = pipeNode->size;
		int r = pipeNode->r;
		uint64_t off = fileDesc->offset;
		// printf("size:%d\n",size);
		// printf("off:%d\n",off);

		if(((r-w)==1 && w!=-1) || (w==(sz-1) && r==0)){
			printf("Buffer is full\n");
			// WAIT
			// return 0;
		}
		if(r==0 && w==-1){	// Empty buffer  // same as (r-w)==1 && w==-1
			if((size-off)<=sz){
				memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),(size-off));
				pipeNode->w = (pipeNode->w + (size-off))%sz;
				fileDesc->offset = fileDesc->offset + (size-off);
				off = fileDesc->offset;
				// printf("w:%d,size:%d\n",pipeNode->w,size);
				// printf("1\n");
				fileDesc->offset = 0;
				return size;
			}else{
				memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),sz);
				pipeNode->w = (pipeNode->w + sz)%sz;
				fileDesc->offset = fileDesc->offset + sz;
				off = fileDesc->offset;
				// WAIT
				// return sz; 
			}
		}
		if(w<r){
			if((size-off)<(r-w)){
				memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),(size-off));
				pipeNode->w = pipeNode->w + (size-off);
				fileDesc->offset = fileDesc->offset + (size-off);
				off = fileDesc->offset;
				// printf("2\n");
				fileDesc->offset = 0;
				return size;
			}else{
				memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),((r-w)-1));
				pipeNode->w = pipeNode->w + (pipeNode->r - pipeNode->w) - 1;	// At this point if w == r - 1 i.e., (r-w)==1 then buffer is full
				fileDesc->offset = fileDesc->offset + ((r-w)-1);
				off = fileDesc->offset;
				// WAIT
				// return ((r-w)-1);
			}
		}else{ 
			if((size-off)<(sz-w+r)){
				if((size-off)<(sz-w)){
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),(size-off));
					pipeNode->w = (pipeNode->w + (size-off))%sz;
					fileDesc->offset = fileDesc->offset + (size-off);
					off = fileDesc->offset;
					// printf("3\n");
					fileDesc->offset = 0;
					return size;
				}else{
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),sz-w-1);
					// fileDesc->offset = fileDesc->offset + (sz-w-1);
					off = off + (sz-w-1);
					w = -1;
					memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),((size-fileDesc->offset)-(sz-w-1)));
					// fileDesc->offset = fileDesc->offset + ((size-off)-(sz-w-1));
					off = off + ((size-off)-(sz-w-1));
					pipeNode->w = (pipeNode->w + (size-(fileDesc->offset)))%sz;
					fileDesc->offset = off;
					// printf("4\n");
					fileDesc->offset = 0;
					return size;
				}
			}else{
				memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),sz-w-1);
				// fileDesc->offset = fileDesc->offset + (sz-w-1);
				off = off + (sz-w-1);
				w=-1;
				memcpy((void *)(pipeNode->pipe_buffer+(w+1)),(void *)(buf+off),r);
				// fileDesc->offset = fileDesc->offset + r;
				off = off + r;
				pipeNode->w = (pipeNode->w + (sz-w-1+r))%sz;
				fileDesc->offset = off;
				//WAIT
				// return (sz-w-1+r);
			}
		}
		printf("[sys_write] Control should never reach here\n");
		return -1;
	}else{
		printf("[sys_write]Something is wrong, invalid fd\n");
		return -1;
	}
}

int sys_close(int fd){
	// if(fd==0 || fd==1 || fd==2){
	// 	current_pcb->fileDescriptors[fd]=NULL;
	// 	return 0;
	// }else{
		struct file_descriptor *fileDesc = current_pcb->fileDescriptors[fd];
		if(fileDesc == NULL){
			printf("Not a valid file descriptor\n");
			return -1;
		}
		fileDesc->refCount--;
		if(fileDesc->refCount == 0){
			if(fileDesc->fdType == TARFILE){
				// 	struct tar_file *temp = fileDesc->file_ptr;
				// FREE(temp);
			}else if(fileDesc->fdType == PIPEWRITE){
				fileDesc->pipe_ptr->isWriteClosed = 1;
			}else if(fileDesc->fdType == PIPEREAD){
				fileDesc->pipe_ptr->isReadClosed = 1;
			}
			current_pcb->fileDescriptors[fd] = NULL;
		}
		return 0;
	// }
}

off_t sys_lseek(int fd, off_t offset, int whence){
	struct file_descriptor *fileDesc = current_pcb->fileDescriptors[fd];
	if(fileDesc == NULL){
		printf("Not a valid file descriptor\n");
		return -1;
	}	

	struct tar_file *filePtr = fileDesc->file_ptr;	
	if(filePtr == NULL){
		printf("Something is wrong! No appropriate file to sys_lseek\n");
		return -1;
	}
	if(filePtr->fileType == DIRTYPE){
		printf("Not valid operation on directory type file\n");
		return -1;
	}	

	if(	whence == SEEK_SET){
		fileDesc->offset = offset;
	}else if(whence == SEEK_CUR){
		fileDesc->offset = fileDesc->offset + offset;
	}else if(whence == SEEK_END){
		fileDesc->offset = filePtr->fileSize + offset;
	}else{
		return -1;
	}
	return fileDesc->offset;
}

int sys_getdents(unsigned int fd, struct dirent *dirp, size_t count){
	struct file_descriptor *fileDesc = current_pcb->fileDescriptors[fd];
	if(fileDesc == NULL){
		printf("Invalid file descriptor\n");
		return -1;
	}
	struct tar_file *filePtr = fileDesc->file_ptr;	
	if(filePtr == NULL){
		printf("Something is wrong! No appropriate file to sys_getdents\n");
		return -1;
	}
	if(filePtr->fileType != DIRTYPE){
		printf("Not valid operation on directory type file\n");
		return -1;
	}
	int index = 0;
	filePtr = filePtr->child;

	while((index!=fileDesc->offset) && filePtr){
		filePtr = filePtr->next;
		index++;
	}
	if(filePtr == NULL){
		fileDesc->offset = 0;
		return 0;
	}

	dirp->d_ino = 0;
	dirp->d_off = 0;
	strncpy(dirp->d_name,filePtr->fileName,NAME_MAX + 1);
	dirp->d_reclen = sizeof(dirp->d_ino) + sizeof(dirp->d_off) + sizeof(dirp->d_reclen) + strlen(dirp->d_name);

	fileDesc->offset++;
	
	return dirp->d_reclen;
}
	
char* sys_getcwd(char *buf, size_t size){
	if(strlen(current_pcb->cwd) >= size){
		printf("Size of buffer is not enough for the cwd path\n");
		return NULL;
	}
	if(size==0 && buf !=NULL){
		printf("Size of buffer is zero");
		return NULL;
	}
	strcpy(buf,current_pcb->cwd);
	return buf;
}

int sys_chdir(const char *path){

	if(strcmp(path,"..")==0){
		char *curDir = (char *)kmalloc(sizeof(char)*MAX_LENGTH_FILENAME);
		strcpy(curDir,current_pcb->cwd);
		struct tar_file *filePtr = lookUpForFile(curDir);
		if(filePtr == NULL){		
			printf("No such file\n");
			return -1;
		}else{
			if(filePtr->parent)
				filePtr = filePtr->parent;
			if(filePtr->fileType != DIRTYPE){
				printf("Path specified is not a directory\n");
				return -1;
			}
			if(filePtr->filePerm != convertOctalToDecimal(755)){
				printf("Permission denied\n");
				return -1;
			}
			strcpy(current_pcb->cwd,filePtr->filePath);
			return 0;
		}
	}else if(strcmp(path,".")==0){
		// Current cwd remains as it is
		return 0;
	}else if((strlen(path)==1) && (path[0]=='/')){   
		strcpy(current_pcb->cwd,tarfsroot->filePath);
		return 0;
	}else{

		char **nodes=NULL;
		int isRelativePath=1;
		int count = 0;
		int i = 0;
		char *curDir = (char *)kmalloc(sizeof(char)*MAX_LENGTH_FILENAME);
        strcpy(curDir,current_pcb->cwd);
		nodes = (char **)kmalloc(sizeof(char *) * MAX_LENGTH_FILENAME);
    	for(int i=0;i < MAX_LENGTH_FILENAME;i++)
        	nodes[i] = NULL;
    	if(path[0] == '/'){
        	isRelativePath=0;
        	path++;
    	}
    	parseFileName(path,nodes,&count);

    	struct tar_file *curNode = NULL;

    	if(isRelativePath){
    		curNode = lookUpForFile(curDir);
    	}else{
    		curNode = tarfsroot;
    	}

    	strcpy(current_pcb->cwd,curNode->filePath);

    	while(count!=0){
    		if(strcmp(nodes[i],"..")==0){
    			if(curNode->parent)
					curNode = curNode->parent;
				if(curNode->fileType != DIRTYPE){
					printf("%s is not a directory\n",nodes[i]);
					strcpy(current_pcb->cwd,curDir);
					return -1;
				}
				if(curNode->filePerm != convertOctalToDecimal(755)){
					printf("Permission denied for %s\n",nodes[i]);
					strcpy(current_pcb->cwd,curDir);
					return -1;
				}
				strcpy(current_pcb->cwd,curNode->filePath);
    		}else if(strcmp(nodes[i],".")==0){

    		}else{
    			curNode = lookUpForFile(nodes[i]);
    			if(curNode == NULL){		
    				strcpy(current_pcb->cwd,curDir);
					printf("No such file\n");
					return -1;
				}else{
					if(curNode->fileType != DIRTYPE){
						printf("%s is not a directory\n",nodes[i]);
						strcpy(current_pcb->cwd,curDir);
						return -1;
					}
					if(curNode->filePerm != convertOctalToDecimal(755)){
						printf("Permission denied for %s\n",nodes[i]);
						strcpy(current_pcb->cwd,curDir);
						return -1;
					}
					strcpy(current_pcb->cwd,curNode->filePath);
    			}
    		}
    		count--;
    		i++;
    	}
    	return 0;
	}
}


int sys_dup(int oldfd){ 						//Shriya
	int i;
	for(i=0;i<=MAX_FILE_DESCRIPTORS_PER_PROCESS;i++){ 
		if(current_pcb->fileDescriptors[i] == NULL){ 
			break;
		} 
	} 
	if(i>MAX_FILE_DESCRIPTORS_PER_PROCESS){ 
		printf("Exceed max file descriptors\n"); 
		return -1; 
	} 
	current_pcb->fileDescriptors[i] = current_pcb->fileDescriptors[oldfd]; 
//	current_pcb->fileDescriptors[i]->file_ptr = current_pcb->fileDescriptors[oldfd]->file_ptr; 
	return i;  
} 

int sys_dup2(int oldfd, int newfd){				//Shriya
	if (current_pcb->fileDescriptors[oldfd]!=NULL)
	{
		if(current_pcb->fileDescriptors[newfd]!=NULL)
		{
			sys_close(newfd);
		}		
		current_pcb->fileDescriptors[newfd] = current_pcb->fileDescriptors[oldfd] ; 
	//	current_pcb->fileDescriptors[newfd]->file_ptr = current_pcb->fileDescriptors[oldfd]->file_ptr; 
		// printf("[sys_dup]:%d\n",newfd);
		return newfd;	
	}
	else
		return -1; 
} 

int sys_pipe(int file_desc[2]){
	int i,j;
	char *buffer = (char *)kmalloc(PIPE_SIZE);
	if(buffer==NULL){
		printf("Out of Memory\n");
		return -1;
	}
	for(i=3;i<=MAX_FILE_DESCRIPTORS_PER_PROCESS;i++){
		if(current_pcb->fileDescriptors[i] == NULL){
			break;
		}
	}
	if(i>MAX_FILE_DESCRIPTORS_PER_PROCESS){
		printf("Exceed max file descriptors\n");
		return -1;
	}
 	struct file_descriptor *fd1 = (struct file_descriptor *)kmalloc(sizeof(struct file_descriptor));
 	if(fd1 == NULL){
 		printf("Out of Memory\n");
 		// FREE buffer
		return -1;
 	}

 	for(j=3;i<=MAX_FILE_DESCRIPTORS_PER_PROCESS;j++){
		if((current_pcb->fileDescriptors[i] == NULL) && j!= i){
			break;
		}
	}
	if(j>MAX_FILE_DESCRIPTORS_PER_PROCESS){
		printf("Exceed max file descriptors\n");
		// FREE fd1 and buffer
		return -1;
	}
 	struct file_descriptor *fd2 = (struct file_descriptor *)kmalloc(sizeof(struct file_descriptor));
 	if(fd2 == NULL){
 		printf("Out of Memory\n");
 		// FREE fd1 and buffer
		return -1;
 	}

 	struct pipe_node *pipeNode = (struct pipe_node *)kmalloc(sizeof(struct pipe_node));
 	if(pipeNode == NULL){
 		printf("Out of Memory\n");
 		// FREE fd1 , fd2 and buffer
		return -1;
 	}
 	pipeNode->r = 0;
 	pipeNode->w = -1;
 	pipeNode->size = PIPE_SIZE;
 	pipeNode->pipe_buffer = buffer;
 	pipeNode->isReadClosed = 0;
 	pipeNode->isWriteClosed = 0;

 	fd1->fdType = PIPEREAD;
 	fd1->pipe_ptr = pipeNode;
	fd1->offset = 0;
	fd1->perm = 0;
	fd1->file_ptr = NULL;
	fd1->refCount = 1;

	fd2->fdType = PIPEWRITE;
 	fd2->pipe_ptr = pipeNode;
	fd2->offset = 0;
	fd2->perm = 0;
	fd2->file_ptr = NULL;
	fd2->refCount = 1;

	current_pcb->fileDescriptors[i] = fd1;
	current_pcb->fileDescriptors[j] = fd2;
	file_desc[0] = i;
	file_desc[1] = j;
	return 0;
}

struct vma* getHeapVMA(struct PCB* process)
{
	struct vma* temp = process->first_vma;
	while(temp)
	{
		if(temp->start==UHEAPSTART)
			return temp;
		temp=temp->next;		
	}
	return temp;
}

void fork_handler(struct PCB* parent, struct regs* parent_regs)
{
	int numpages=0,i=0;
	struct regs * child_regs=NULL;
	struct page* newpage=NULL;
	struct PCB* child = pcb_alloc();
	struct PCB* runner=NULL;
	struct vma* vma_temp=NULL;
	struct vma* new_child_vma=NULL;
	struct vma* prev_child_vma=NULL;
	uint64_t *ret_val;	
	


	//Create new process and copy contents
	//NEW 7th MAY: AADY
	child->parentPCB = parent;
	if(parent->isFore)
	{
		child->isFore=1;
		parent->isFore=0;
	}

	child->reg = parent->reg; 
	child->processID = generate_pid();
	child->parentProcessID = parent->processID;
	child->fileName = kmalloc(128);
	strcpy(child->fileName, parent->fileName);
	child->cwd = kmalloc(128);
	memset(child->cwd,0,strlen(parent->cwd)+1);
	if((strlen(parent->cwd)==1) && (parent->cwd[0]=='/')){
		*(child->cwd) = '/';
		*(child->cwd+1) = '\0';
	}else	
		strcpy(child->cwd,parent->cwd);
	child->time=0;
	for(int i=0; i<MAX_FILE_DESCRIPTORS_PER_PROCESS;i++){
		if(parent->fileDescriptors[i]==NULL)
			child->fileDescriptors[i]=NULL;
		else{
			parent->fileDescriptors[i]->refCount++;
			child->fileDescriptors[i] = parent->fileDescriptors[i];
		}
	}
		
	//OTHER FIELDS PENDING
	
	//Allocate a page for pml4 of child, copy entries
	newpage = page_alloc();
	child->pml4= (uint64_t *)getVA(newpage);
	child->cr3= (uint64_t *)getPA(newpage);
	
	//Copying kernel entries
	child->pml4[PML4OFF(KERNBASE)] = parent->pml4[PML4OFF(KERNBASE)];	


	//Mark page table entries in parent as COW and R only
	//Traverse VMAs for this
	vma_temp = parent->first_vma;
	while(vma_temp)
	{
		//Create vmas for child			
		new_child_vma = vma_alloc();	
		if(vma_temp == parent->first_vma)
		{
			child->first_vma = new_child_vma;
			*new_child_vma=*vma_temp; //Copies contents to this
			prev_child_vma = new_child_vma;
			new_child_vma->next=NULL;
			new_child_vma->prev=NULL;	
		}
		else
		{
			*new_child_vma=*vma_temp;
			prev_child_vma->next=new_child_vma;
			new_child_vma->prev=prev_child_vma;
			new_child_vma->next=NULL;
			prev_child_vma=new_child_vma;		
		}
	
		/*
		if(vma_temp->flags_vma & 0x2) //Writable memory
		{
			numpages = ((uint64_t)alignUp((void *)vma_temp->end) - (uint64_t)alignDown((void *)vma_temp->start))/PAGE_SIZE;
			for(i=0;i<numpages;i++)
			{
				ret_val = pml4Walk(parent->pml4, vma_temp->start + i*PAGE_SIZE);
				if(*ret_val)
				{
					*ret_val = (*ret_val & ~(0x2)) | BIT_COW;					
					//Get page of this phy addr and copy it to child's page table entries
					newpage = getStrPtrFromPA((void *)*ret_val);			
					
					//NOTE: page_insert below will inc ref count of the page to 2.
					page_insert(child->pml4,(void *)(vma_temp->start + i*PAGE_SIZE), newpage, BIT_PRESENT | BIT_USER | BIT_COW);
				}					
			}	
	
		}
		else //Cannot Share page tables, can share pages
		{
			
			numpages = ((uint64_t)alignUp((void *)vma_temp->end) - (uint64_t)alignDown((void *)vma_temp->start))/PAGE_SIZE;
			for(i=0;i<numpages;i++)
			{
				ret_val = pml4Walk(parent->pml4, vma_temp->start + i*PAGE_SIZE);
				if(*ret_val)
				{	
					//Get page of this phy addr and copy it to child's page table entries
					newpage = getStrPtrFromPA((void *)*ret_val);			
					
					//NOTE: page_insert below will inc ref count of the page to 2.
					page_insert(child->pml4,(void *)(vma_temp->start + i*PAGE_SIZE), newpage, BIT_PRESENT | BIT_USER);
				}					
			}


		}	
		
		*/
		numpages = ((uint64_t)alignUp((void *)vma_temp->end) - (uint64_t)alignDown((void *)vma_temp->start))/PAGE_SIZE;
		for(i=0;i<numpages;i++)
		{
			ret_val = pml4Walk(parent->pml4, vma_temp->start + i*PAGE_SIZE);
			if(*ret_val)
			{	
									
				//Get page of this phy addr and copy it to child's page table entries
				newpage = getStrPtrFromPA((void *)*ret_val);

				if(vma_temp->flags_vma & 0x2) //Writeable memory
				{	
					*ret_val = (*ret_val & ~(0x2)) | BIT_COW;
					page_insert(child->pml4,(void *)(vma_temp->start + i*PAGE_SIZE), newpage, BIT_PRESENT | BIT_USER | BIT_COW);
				}	
				else //Readonly memory
				{
					page_insert(child->pml4,(void *)(vma_temp->start + i*PAGE_SIZE), newpage, BIT_PRESENT | BIT_USER);
				}		
			}
		}
				

		vma_temp = vma_temp->next;
	}

	//Reload CR3 to flush out old entries out of TLB: VERY IMPORTANT
	loadcr3(current_pcb->cr3);	

	
	//Copying exception stack from parent to child
	child->ersp = UXSTACKTOP - (child->processID*PAGE_SIZE);
	memcpy((void *)(child->ersp-PAGE_SIZE+1), (void *)(parent->ersp-PAGE_SIZE+1), PAGE_SIZE);	//CHECK
		
	
	//Setting return values on stack
	parent_regs->rax = child->processID; //Return child pid in parent
	parent->reg.rax = child->processID;


	//********************CHECK****************		
	child_regs = (struct regs *)(child->ersp - (parent->ersp - (uint64_t)parent_regs));
	
	//rsp value at  child_regs points to rsp of parent, change it
	child_regs->ex_rsp = parent_regs->ex_rsp - (parent->ersp - child->ersp);
	
	//Setting up child exception stack pointer
	child->reg.ex_rsp = (uint64_t)child_regs; //CHECK	

	//Return values in child process
	child_regs->rax = 0;	//Return 0 in child		
	child->reg.rax = 0;		

	
	//Other initialization
	child ->hasRan = 1; //Dont want schedule() to call build_addr_space
	
	//Add to list of runnable processes
	runner=current_pcb;
	while(runner->next)
	{
		runner=runner->next;	
	}
	runner->next=child;	
	child->prev=runner; 
	child->next=NULL;	

	// printf("exiting fork handler.\n");
}//end of fork_handler()




uint64_t alignUpEight(uint64_t temp)
{
	
	while(temp % 8)
	{
		temp++;
	} 
	return temp;
}

uint64_t alignDownEight(uint64_t temp)
{
	while(temp % 8)
	{
		temp--;
	} 
	return temp;
}


//Execve handler
void execve_handler(struct PCB* process, struct regs * r) {
	char *newfileName = (char *)r->rdi;
	
	//printf("newfileName:%s\n", newfileName);
	if(!checkELF(newfileName)) {
		r->rax = -1;
		return;
	}
	//struct PCB *temp1;
	struct PCB *temp;
	//int wPID;
	int isAmpersend = 0;

	struct page* newpage=NULL;
	uint64_t *tempval;
	int counter1=0,counter2=0;
	int i=0;
	uint64_t *userstack=NULL;
	uint64_t *write_start=NULL;
	uint64_t offset=0;
	//int fileNameSize = strlen(newfileName);

	//Delete old file name
	memset((void *)process->fileName, '\0', strlen(process->fileName));
	
	/*
	if ((newfileName[fileNameSize - 1] == '&') && (newfileName[fileNameSize - 2] == ' '))
	{
		printf("Ampersand here true\n");
		while(1);
		isAmpersend = 1;
		newfileName[fileNameSize - 1] = 0;
		newfileName[fileNameSize - 2] = 0;
	} else {
		isAmpersend = 0;
	}
	*/
	//Convert relative filename to absolute
	/*if((!strncmp(newfileName, "bin", 3)) && (*(newfileName+3) == '/'))
	{
		strcpy(process->fileName, newfileName);
	}
	else
	{
		strcpy(process->fileName,"bin");
		*(process->fileName + 3) = '/';
		strcpy((char *)(((uint64_t)process->fileName)+4), newfileName);
	}*/
	strcpy(process->fileName, newfileName);


	while(*((uint64_t *)r->rsi+i)!='\0')
	{
		//printf("%x , ", ((uint64_t *)r->rsi+i));
		counter1++;
		i++;
	}	
	i=0;
	while(*((uint64_t *)r->rdx+i)!='\0')
	{
		//printf("%x , ", ((uint64_t *)r->rdx+i));
		counter2++;
		i++;
	}	

	//printf("argc: %d\n",counter1);
	
	//printf("counter1: %d\n", counter1);
	//printf("counter2: %d\n", counter2);
	//*********************IMPORTANT*****************************
	//ACCESSING USER STACK USING KERNEL ADDRESSES
	newpage = page_alloc();
	userstack = (uint64_t *)(getVA(newpage)+PAGE_SIZE-0x8);
	//printf("kernel userstack addr: %x\n",userstack);
	
	//SET UP STACK

	//Calculating Size requirements
	for(i=counter2-1;i>=0;i--)
	{
		//printf("%s\n",(char *)*((uint64_t *)r->rdx+i));
		offset = offset + alignUpEight(strlen((char *)(*((uint64_t *)r->rdx+i))) + 1)/8;
	}
	offset++; //For null write after envp

	//printf("Printing argvs from execve_handler: \n");
	for(i=counter1-1;i>=0;i--)
	{	
		//printf("%s\n",(char *)*((uint64_t *)r->rsi+i));
		if((*((char *)*((uint64_t *)r->rsi+i))=='&'))
		{
			//printf("amersand true\n");
			isAmpersend=1;
			counter1--;
		}
		offset = offset + alignUpEight(strlen((char *)(*((uint64_t *)r->rsi+i))) + 1)/8;
	}
	offset++; //For null write after argv

	write_start = userstack-offset-1;
	// printf("write_start=%x\n",write_start);

	for(i=counter2-1;i>=0;i--)
	{
		userstack = userstack - alignUpEight(strlen((char *)(*((uint64_t *)r->rdx+i))) + 1)/8;
		strcpy((char *)userstack,(char *)(*((uint64_t *)r->rdx+i)));
		
		*write_start = (uint64_t)USTACKTOP - ((uint64_t)alignUp((void *)userstack) - (uint64_t)userstack);
		//printf("%x: %x\n",write_start,userstack);
		write_start--;
	}


	*write_start='\0';
	// printf("write_start for null write: %x\n",write_start);
	write_start--;

	*(--userstack)='\0';
	
	for(i=counter1-1;i>=0;i--)
	{
		userstack = userstack - alignUpEight(strlen((char *)(*((uint64_t *)r->rsi+i))) + 1)/8;
		
		strcpy((char *)userstack,(char *)(*((uint64_t *)r->rsi+i)));
		
		*write_start = (uint64_t)USTACKTOP - ((uint64_t)alignUp((void *)userstack) - (uint64_t)userstack);
		//printf("%x: %x\n",write_start,userstack);
		write_start--;
	}
	*write_start = counter1;
	//printf("%x: %x\n",write_start,*write_start);

	*(--userstack)='\0';
	//printf("userstack after all : %x\n", userstack);


	

	vma_remove_for_process(process);

	//Call build_process_address_space with modifications
	build_process_address_space(process); //NOTE: hasRan will be updated to zero inside.

	//Change values of rip and rsp on exception stack for iretq to run new process
	tempval = (uint64_t *)&r->rdi;
	tempval = tempval+0x2;
	
	//Change rip
	*tempval = process->reg.rip; //updated in build_process_address_space

	//Change stack
	offset = (uint64_t)alignUp((void *)write_start) - (uint64_t)write_start;
	*(tempval+0x3) = (USTACKTOP-offset); 
	page_insert(process->pml4, (void *)(USTACKTOP-PAGE_SIZE), newpage, BIT_USER | BIT_PRESENT | BIT_RW);

	if(!isAmpersend) {

		process -> isFore = 1;
		if(process->parentPCB)
			if(!process->parentPCB->hasExited)
				(process -> parentPCB) -> isFore = 0;
		
			/*
		wPID = process -> processID;
		temp = wait_pcb_head;
		temp1 = process -> parentPCB;

	
		if (temp1 == pcb_head) if (temp1 -> next != NULL) pcb_head = temp1 -> next;
		while (temp != NULL) {
			if (!temp -> next) break;
			temp = temp -> next;
		}
		printf("here 1\n");
		if (temp1 -> next != NULL) (temp1 -> next) -> prev = temp1 -> prev;
		if (temp1 -> prev != NULL) (temp1 -> prev) -> next = temp1 -> next;
		if (wait_pcb_head != NULL) {
			temp -> next = temp1;
			temp1 -> prev = temp;
			temp1 -> next = NULL;
			temp1 -> waitPID = wPID;
		} else {
			wait_pcb_head = temp1;
			wait_pcb_head -> next = NULL;
			wait_pcb_head -> prev = NULL;
			wait_pcb_head -> waitPID = wPID;
		}*/
	}
	else //Ampersand process 
	{
		//printf("here Ampersand\n");
		if(process->parentPCB && !process->parentPCB->hasExited)
		{
			process -> isFore = 0;
			(process -> parentPCB) -> isFore = 1;

			current_pcb->reg.ex_rsp = (uint64_t)r;

			//TEST
			//Put parent from wait queue to runnable again.
			temp = wait_pcb_head;
			while(temp)
			{
				if(temp==process->parentPCB)
					break;
				temp=temp->next;
			}
			if(temp)
			{
				//printf("temp=%s\n",temp->fileName );
				if(temp==wait_pcb_head)
				{
					if(temp->next)
						wait_pcb_head=temp->next;
					else
						wait_pcb_head=NULL;		
				}
				if(temp->prev)
					temp->prev->next = temp->next;
				if(temp->next)
					temp->next->prev = temp->prev;

				temp->next=current_pcb->next;
				temp->prev=current_pcb;
				current_pcb->next=temp;
				if(temp->next)
					temp->next->prev=temp;
				current_pcb=temp;

				loadcr3(current_pcb->cr3);//Flush out tlb
				SchNext();
			}
				

		}	
		else
			process->isFore=1;
	}
	//while(1);
	//RELOAD CR3 AT END

	loadcr3((void *)process->cr3); //Need to flush out tlb while accessing userstack with kernel address
}



void syscall_handler(struct regs *r)
{
	uint64_t syscallNo = r->rax;

//	printf("int number:%d\n",r.int_number);		
//	printf("syscall no: %d\n",syscallNo);
		
//	printf("rdi:%d\n",(unsigned int)r.rdi);
//	printf("rsi:%s\n",(const char *)r.rsi);
//	printf("rdx:%d\n",(size_t)r.rdx);

	switch(syscallNo){

		case SYS_write:{
			ssize_t ret = sys_write((int)r->rdi,(const void *)r->rsi,(size_t)r->rdx, r);	
			r->rax = ret;	
			break;
		}
		case SYS_sched_yield:{
			//yield(r);
			break;
		}
		case SYS_exit:{
			//printf("Exit for: %s\n",current_pcb->fileName);
			sys_exit(current_pcb);
			break;
		}
		case SYS_brk:{
			struct vma* vma_temp=NULL;
			uint64_t new_heap_top = (uint64_t)r->rdi;
			vma_temp = getHeapVMA(current_pcb);
			
			if(!r->rdi)
			{
				r->rax = vma_temp->end;	
			}
			else
			{
				if(new_heap_top > UHEAPLIMIT || new_heap_top < UHEAPSTART)
				{
					r->rax = -1; //Implying failure
				}
				else
				{
					r->rax = 0;
					if(new_heap_top > vma_temp->end)	
					{
						vma_temp->end = (uint64_t)alignUp((void *)new_heap_top);
					}
				}		
			}
			break;
		}
		case SYS_fork:{
			//aady : 24th apr	
			fork_handler(current_pcb, r);			
			break;
		}
		case SYS_getpid:{
			r -> rax = current_pcb->processID;
			break;
		}
		case SYS_getppid:{
			r -> rax = current_pcb->parentProcessID;
			break;
		}
		case SYS_execve:{
			execve_handler(current_pcb, r);
			break;
		}
		case SYS_wait4:{
			
			if(r->rdi)
			{
				//waitPlease(current_pcb, r, (int) r -> rdi);	
				waitPlease(r, (int) r -> rdi);	
			}
			else
				printf("Cannot wait on idea process.\n");
			
			break;
		}
		case SYS_nanosleep:{
			struct timespec_users
			{
				signed long sec;
				long nsec;
			};
			struct timespec_users *tSec = (void *) r -> rdi; // I have no idea how to retrive
			unsigned int timeSec = tSec -> sec;
			sleepPlease(r, timeSec);
			break;
		}
		case SYS_alarm:{
			sys_exit(current_pcb);
			break;
		}
		case SYS_getcwd:{
			char *ret = sys_getcwd((char *)r->rdi,(size_t)r->rsi);
			r->rax = (uint64_t)ret;
			break;
		}
		case SYS_chdir:{
			int ret = sys_chdir((char *)r->rdi);
			r->rax = ret;
			break;
		}
		case SYS_open:{
			int ret = sys_open((const char *)r->rdi,(int)r->rsi);
			r->rax = ret;
			break;
		}
		case SYS_read:{
			ssize_t ret = sys_read((int)r->rdi, (void *)r->rsi, (size_t)r->rdx, r);
			r->rax = ret;
			break;
		}
		case SYS_lseek:{
			off_t ret = sys_lseek((int)r->rdi, (off_t)r->rsi, (int)r->rdx);
			r->rax = ret;
			break;
		}
		case SYS_close:{
			int ret = sys_close((int)r->rdi);
			r->rax = ret;
			break;
		}
		case SYS_pipe:{
			 int ret = sys_pipe((int *)r->rdi);
			 r->rax = ret;
			break;
		}
		case SYS_dup:{
			int ret = sys_dup((int)r->rdi);
			r->rax = ret;
			break;
		}
		case SYS_dup2:{
			int ret = sys_dup2((int)r->rdi,(int)r->rsi);
			r->rax = ret;
			break;
		}
		case SYS_getdents:{
			int ret = sys_getdents((unsigned int)r->rdi,(struct dirent *)r->rsi,(size_t)r->rdx);
			r->rax = ret;
			break;
		}
		case SYS_listPCB:{
			printf("Process ID      Time (Sec.)     File Name\n\n");
			PSisCalled(pcb_head);
			PSisCalled(sleep_pcb_head);
			PSisCalled(wait_pcb_head);
			PSisCalled(block_pcb_head);
			break;
		}
		case SYS_killPCB:{
			uint64_t killPID = (uint64_t) r->rdi;
			if(killPID)
			{
				if((KillisCalled(pcb_head, killPID)) || (KillisCalled(sleep_pcb_head, killPID)) || (KillisCalled(wait_pcb_head, killPID)) || (KillisCalled(block_pcb_head, killPID))) printf("Process Killed\n"); else printf("ERROR!! Process NOT Killed: Wrong Process ID\n");
			}
			else
			{
				printf("Cannot kill ideal process. Kill Failed.\n");
			}
			break;
		}
		default: break;
	}
}
