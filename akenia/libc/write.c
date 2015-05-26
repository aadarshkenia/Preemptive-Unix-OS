#include <stdlib.h>
#include <sys/syscall.h>

ssize_t write(int fd, const void *buf, size_t size)
{
    ssize_t ret;
    __asm__ volatile
    (
        "syscall"
        : "=a" (ret)
        : "0"(SYS_write), "D"(fd), "S"(buf), "d"(size)
        : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
