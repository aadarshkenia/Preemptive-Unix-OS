#include <stdlib.h>
#include <sys/syscall.h>

int getdents(unsigned int fd, struct dirent *dirp, size_t count)
{
 	int ret;
        __asm__ volatile
           (
        "int $0x80;"
        : "=a" (ret)
        : "0"(SYS_getdents), "D"(fd), "S"(dirp), "d"(count)
        : "cc", "rcx", "r11", "memory"
    );
    return ret;
}
