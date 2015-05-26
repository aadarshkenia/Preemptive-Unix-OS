#include <stdlib.h>
#include <sys/syscall.h>

int dup2(int old_fd, int new_fd)
{
    int ret;
    __asm__ volatile
    (
    "int $0x80;"
    : "=a" (ret)
    : "0"(SYS_dup2), "D"(old_fd), "S"(new_fd)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
