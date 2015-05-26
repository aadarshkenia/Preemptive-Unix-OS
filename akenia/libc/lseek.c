#include <stdlib.h>
#include <sys/syscall.h>

off_t lseek(int fileds, off_t offset, int whence)
{
        off_t ret;
        __asm__ volatile
        (
        "syscall"
        : "=a"(ret)
        : "0"(SYS_lseek), "D"(fileds), "S"(offset), "d"(whence)
        : "cc", "rcx", "r11", "memory"
        );
        return ret;

}
