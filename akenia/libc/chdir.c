#include <stdlib.h>
#include <sys/syscall.h>

int chdir(const char * path)
{
    int ret;
    __asm__ volatile
    (
    "syscall"
    : "=a" (ret)
    : "0"(SYS_chdir), "D"(path)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
