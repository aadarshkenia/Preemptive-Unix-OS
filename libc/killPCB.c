#include <stdlib.h>
#include <sys/syscall.h>

void KillPCB(uint64_t pid) {
	uint64_t ret;
    __asm__ volatile
    (
    "int $0x80;"
    : "=a"(ret)
    : "0"(SYS_killPCB), "D"(pid)
    : "cc", "rcx", "r11", "memory"
    );
}
