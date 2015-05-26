#include <stdlib.h>
#include <sys/syscall.h>

pid_t fork()
{
    pid_t ret;
    __asm__ volatile
    (
    "syscall"
    : "=a" (ret)
    : "0"(SYS_fork)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
