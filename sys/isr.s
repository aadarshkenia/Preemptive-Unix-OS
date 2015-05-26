
.text

.extern isr_handler
.extern syscall_handler

.global isr0
.global isr13
.global isr14
.global isr128
.global isr_common

isr0:
 	cli
 	pushq $0
    pushq $0
    pushq %rdi
    pushq %rsi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rsp
    movq %rsp,%rdi
    callq isr_handler
    popq %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rsi
    popq %rdi
    popq %rax 
    add $0x10,%rsp
    sti
    iretq
    
isr13:
 	cli
# 	pushq $0 #changed aady: 23rd apr
    pushq $13
    pushq %rdi
    pushq %rsi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rsp
    movq %rsp,%rdi
    callq isr_handler
    popq %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rsi
    popq %rdi
    add $0x10,%rsp
    sti
    iretq
    
isr14:
 	cli
# 	pushq $0
    pushq $14
    pushq %rdi
    pushq %rsi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rsp
    movq %rsp,%rdi
    callq isr_handler
    popq %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rsi
    popq %rdi
    add $0x10,%rsp
    sti
    iretq

isr128:
    cli
   pushq $128
    pushq %rdi
    pushq %rsi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rsp  
    movq %rsp, %rdi
    callq syscall_handler
    popq %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rsi
    popq %rdi
    add $0x08, %rsp
    sti
    iretq

isr_common:
	cli
	pushq %rdi
	pushq %rsi
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    pushq %rsp
    movq %rsp,%rdi
    callq isr_handler
    popq %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    popq %rsi
    popq %rdi
    #add $0x10, %rsp
    sti
    iretq

