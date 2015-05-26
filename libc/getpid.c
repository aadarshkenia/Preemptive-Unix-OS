#include <stdlib.h>
#include <sys/syscall.h>

pid_t getpid()
{
    pid_t ret;
    __asm__ volatile
    (
    "int $0x80;"
    : "=a" (ret)
    : "0"(SYS_getpid)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
