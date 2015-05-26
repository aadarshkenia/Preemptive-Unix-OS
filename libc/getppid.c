#include <stdlib.h>
#include <sys/syscall.h>

pid_t getppid()
{
    pid_t ret;
    __asm__ volatile
    (
    "int $0x80;"
    : "=a" (ret)
    : "0"(SYS_getppid)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
