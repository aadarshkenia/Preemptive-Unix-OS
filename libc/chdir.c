#include <stdlib.h>
#include <sys/syscall.h>

int chdir(const char * path)
{
    int ret;
    __asm__ volatile
    (
    "int $0x80;"
    : "=a" (ret)
    : "0"(SYS_chdir), "D"(path)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
