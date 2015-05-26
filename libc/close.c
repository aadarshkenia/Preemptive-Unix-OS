#include <stdlib.h>
#include <sys/syscall.h>

int close(int fd)
{
    int ret;
    __asm__ volatile
    (
        "int $0x80;"
        : "=a" (ret)
        : "0"(SYS_close), "D"(fd)
        : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
