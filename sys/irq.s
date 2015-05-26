
#CHANGED
.text

.extern irq_handler
.extern timer_handler
.extern keyboard_handler
.global irq0
.global irq1
.global irq_common


irq0:
	cli
	#pushq $0
	pushq $32
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
	callq timer_handler
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
	sti #sti uncommented
	iretq


irq1:
	cli
	#pushq $0
	pushq $33	
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
	callq keyboard_handler
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

irq_common:
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
	movq %rsp, %rdi
	callq irq_handler
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
