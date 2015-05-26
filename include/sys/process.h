#ifndef _PROCESS_H_
#define _PROCESS_H_
 
#include <sys/sbunix.h>
#include <sys/structs.h>
 
//Build process address space: called by kernel while creating process
void build_process_address_space(struct PCB * process);
int generate_pid();
//4th May
void create_process(char *filename, int foreground); //Process creation: pass 1 as second param is a foreground process
//extern void createProcess(PCB *, void(*)(), uint64_t, uint64_t*);

#endif
