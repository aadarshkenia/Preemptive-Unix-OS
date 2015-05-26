#include <stdlib.h>
#include <sys/syscall.h>

off_t lseek(int fileds, off_t offset, int whence)
{
        off_t ret;
        __asm__ volatile
        (
        "int $0x80;"
        : "=a"(ret)
        : "0"(SYS_lseek), "D"(fileds), "S"(offset), "d"(whence)
        : "cc", "rcx", "r11", "memory"
        );
        return ret;

}
