#include <stdlib.h>
#include <sys/syscall.h>

int getdents(unsigned int fd, void *dirp, size_t count)
{
 	int ret;
        __asm__ volatile
           (
        "syscall"
        : "=a" (ret)
        : "0"(SYS_getdents), "D"(fd), "S"(dirp), "d"(count)
        : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
