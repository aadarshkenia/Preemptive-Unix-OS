#include <stdlib.h>
#include <sys/syscall.h>

pid_t fork()
{
    pid_t ret;
    __asm__ volatile
    (
    "int $0x80;"
    : "=a" (ret)
    : "0"(SYS_fork)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
