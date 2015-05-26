#include <stdlib.h>
#include <sys/syscall.h>

ssize_t read(int fd, void *buf, size_t count)
{
    int ret;
    __asm__ volatile
    (
        "syscall"
        : "=a" (ret)
        : "0"(SYS_read), "D"(fd), "S"(buf), "d"(count)
        : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
