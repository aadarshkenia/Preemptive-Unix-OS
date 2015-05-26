#include <stdlib.h>
#include <sys/syscall.h>

void exit(int status)
{
        __asm__ volatile
        (
        "syscall"
        ://:
        : "a"(SYS_exit), "D"(status)
        : "cc", "rcx", "r11", "memory"
        );
        return;
}
