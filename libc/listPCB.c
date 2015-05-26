#include <stdlib.h>
#include <sys/syscall.h>

void listPCB() {
	pid_t ret;
    __asm__ volatile
    (
    "int $0x80;"
    : "=a"(ret)
    : "0"(SYS_listPCB)
    : "cc", "rcx", "r11", "memory"
    );
}