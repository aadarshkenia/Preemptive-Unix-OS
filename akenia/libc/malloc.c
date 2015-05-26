#include <stdlib.h>
#include <sys/syscall.h>

void * heap_end;
int brk(void * end_of_segment);
void * sbrk(int increment);


struct Node {
        unsigned long size;
        int isFree;
        struct Node *nextNode;
};

void *head = NULL;

struct Node *spaceNeeded(struct Node* last, unsigned long size) {
        struct Node *block;
        block = sbrk(0);
        if (sbrk(size + sizeof(struct Node)) == (void*) -1) return NULL;
        if (last) last->nextNode = block;
        block->size = size;
        block->nextNode = NULL;
        block->isFree = 0;
        return block;
}

void *malloc(unsigned long size) {
	brk(0);
        struct Node *block;
        if (!head) {
                block = spaceNeeded(NULL, size);
                if (block) head = block;
        } else {
                struct Node *last = head;
                struct Node *present = head;
                while (present && (!(present->isFree) || (present->size < size))) {
                        last = present;
                        present = present->nextNode;
                }
                block = present;
                if (!block) block = spaceNeeded(last, size);
                else block->isFree = 0;
        }
        return(block + 1);
}

void free(void *ptr) {
      if (!ptr) return;
//      printf("\nPointer is - %x",ptr);
((struct Node*) (ptr - sizeof(struct Node)))->isFree = 1;
}
void * sbrk(int increment)
{
        void * oldval = heap_end;
        void * newval = oldval + increment;

 //     if(!brk(newval))
   //           return oldval;
        //else
//printf("error sbrk");
        brk(newval);

      return oldval;
}

int brk(void *end_data_segment)
{

    void * ret;

        __asm__ volatile
    (
        "syscall"
        : "=a" (ret)
        : "0"(SYS_brk), "D"(end_data_segment)
        : "cc", "rcx", "r11", "memory"
    );
heap_end = ret;
    return 0;
}

