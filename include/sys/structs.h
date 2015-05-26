#ifndef _STRUCTS_H
#define _STRUCTS_H
//CHANGED

#include <sys/sbunix.h>
#include <sys/irq_isr_common.h>

#define MAX_FILE_DESCRIPTORS_PER_PROCESS 15

//24 bytes each after alignment
struct page {
	struct page	*next;
	struct page	*prev;
	//uint64	*start_address;
	int			ref_count;
	short		flags_page;
};

typedef enum {
	REGTYPE = 0,
	DIRTYPE = 5
}FILE_TYPE;

typedef enum {
	TARFILE = 1,
	PIPEREAD = 2,
	PIPEWRITE = 3,
	STDFILETYPE = 4
}FD_TYPE;

//40 bytes each
struct vma {
	struct vma	*next;
	struct vma	*prev;
	uint64_t	start; 
	uint64_t	end;
	unsigned long	flags_vma; //Understanding the linux kernel book
};

//20 bytes each
struct mm_struct {
	struct vma	*first_vma;
	int			count; //No of vma s
};
 
 
struct PCB {
	uint64_t	*pml4; 
	uint64_t	*cr3;//Needed? Confirm.
	struct regs	reg;
	int			processID;
	int			parentProcessID;
	//AADY: 10th APR
	struct vma	*first_vma; //will be later on pointed to by mm_struct
	int			hasRan; //17th APR Aaswad
	struct PCB	*next;
	struct PCB	*prev;
	uint64_t	ersp;
	char		*fileName;
	size_t		sleepTimeLeft;
	unsigned long time; // shriya
	int			waitPID;
	int			isFore;
	// kavana
	struct file_descriptor *fileDescriptors[MAX_FILE_DESCRIPTORS_PER_PROCESS];
	char *cwd;

	//NEW 7th MAY
	int hasExited;
	struct PCB* parentPCB;
};

struct tar_file{
	struct tar_file *next;
	struct tar_file *child;
	struct tar_file *parent;
	char *fileName;	
	char *filePath;
	uint64_t fileSize;
	uint64_t *start;
	uint64_t *end;
	FILE_TYPE fileType;
	uint64_t filePerm;
};

struct pipe_node{
	int r;
	int w;
	char *pipe_buffer;
	uint64_t size;
	uint64_t isWriteClosed;
	uint64_t isReadClosed;
};

struct file_descriptor{
	FD_TYPE fdType;
	uint64_t offset;
	uint64_t perm;
	uint64_t refCount;
	struct tar_file *file_ptr;
	struct pipe_node *pipe_ptr;
};

#endif
