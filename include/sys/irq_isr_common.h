#ifndef _IRQ_ISR_COMMON_H
#define _IRQ_ISR_COMMON_H

#include <sys/defs.h>

/*struct regs
{
	uint64_t ex_rsp, r15, r14, r13, r12, r11, r10, r9, r8; 
	uint64_t rdi, rsi, rbp, rdx, rcx, rbx, rax;
    uint64_t int_number, err_code;    
    uint64_t rip, cs, rflags, userrsp, ss;   
};
*/

struct regs
{
        uint64_t ex_rsp, r15, r14, r13, r12, r11, r10, r9, r8;
        uint64_t rbp, rdx, rcx, rbx, rax, rsi, rdi;
    uint64_t int_number, err_code;
    uint64_t rip, cs, rflags, userrsp, ss;
};

#endif
