#include <stdlib.h>
#include <sys/syscall.h>

int pipe(int file_desc[2])
{
    int ret;
    __asm__ volatile
    (
    "syscall"
    : "=a"(ret)
    : "0"(SYS_pipe), "D"(file_desc)
    : "cc", "rcx", "r11", "memory"
    );

    return ret;
}
