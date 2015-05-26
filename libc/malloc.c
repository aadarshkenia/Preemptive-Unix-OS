#include <stdlib.h> 
#include <string.h> 
#include <sys/syscall.h> 
#include <stdio.h> 
#define n 500 //need to optimize

//void * heap_end; 
uint64_t brk(void * end_of_segment); 
void * sbrk(size_t increment); 
int index = 0;

struct node { 
        unsigned long size; 
        uint64_t mem_ptr;
    int isFree; 
        };

struct node nodes[n];

//SIMPLER MALLOC 
void * malloc(unsigned long size) 
{ 
unsigned long bestfit = 100000000;
int bestindex=0;
int flag = 0;
int i;
for (i=0;i<n;i++)
{
if (nodes[i].isFree == 1 && nodes[i].size>=size)
    {
    flag= 1;    
    if ( nodes[i].size < bestfit)
        {
        bestfit =  nodes[i].size;
        bestindex = i;
        }
    }
}
if (flag ==0)     //new space is malloced on top of heap
    { 
    static uint64_t heap_end = 0;
    uint64_t temp=heap_end; 

            if(!heap_end)
    {   
                heap_end = brk(0);
        temp=heap_end;
    }   
        if(!brk((void *)(heap_end+size)))
            { 
            temp=heap_end; 
            heap_end=heap_end+size; 
             memset((void *)temp,0,size); 
                nodes[index].size = size;
        nodes[index].isFree=0;
        nodes[index].mem_ptr = temp;
        index++;
        return (void *)temp;   
            }    
        return (void *)0; 
}
else            //reallocating
{
nodes[bestindex].isFree = 0;
memset((void *)nodes[bestindex].mem_ptr,0,size); 
return (void *)nodes[bestindex].mem_ptr;

}
return (void *)0;
}


void free(void * mem_ptr) { 
    if (!mem_ptr) return; 
      // printf("Pointer is - %x\n",mem_ptr);  
    int i;
    for (i=0;i<n;i++)
    {
        if ((void *) nodes[i].mem_ptr == mem_ptr)
            nodes[i].isFree =1;
    }
}


void * sbrk(size_t increment) 
{ 
    if(increment <0) 
    { 
        printf("Wrong increment size. \n"); 
        return (void *)-1; 
    } 
       uint64_t oldval = brk(0); 
       uint64_t newval = oldval + increment; 

       if(brk((void *)newval)<0) 
    { 
        printf("Error while incrementing inside sbrk\n"); 
                return (void *)-1; // (void *)oldval; 
        }//else 

        return (void *)oldval; //Return start addr of allocated segment 
} 

uint64_t brk(void *end_data_segment) 
{ 
uint64_t ret; 
    __asm__ volatile 
    ( 
        "int $0x80;" 
        : "=a" (ret) 
        : "0"(SYS_brk), "D"(end_data_segment) 
        : "cc", "rcx", "r11", "memory" 
   ); 
//heap_end = (void *)ret; 
    return ret; 
} 


/*#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <stdio.h>


//SIMPLER MALLOC
void * malloc(unsigned long size)
{
	static uint64_t heap_end=0;
    uint64_t temp=heap_end;
	if(!heap_end)
    {
		heap_end = brk(0);
        temp=heap_end;
    }
	if(!brk((void *)(heap_end+size)))
	{
        temp=heap_end;
        heap_end=heap_end+size;
	
	//3rd MAY
	//to map the page belonging to this mem
	memset((void *)temp,0,size);

        return (void *)temp;   
    }		
	return (void *)0;
}

uint64_t brk(void *end_data_segment)
{

uint64_t ret;
    __asm__ volatile
    (
        "int $0x80;"
        : "=a" (ret)
        : "0"(SYS_brk), "D"(end_data_segment)
        : "cc", "rcx", "r11", "memory"
    );
//heap_end = (void *)ret;
    return ret;
}*/
