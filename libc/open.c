#include <stdlib.h>
#include <sys/syscall.h>

int open(const char *buf, int flags)
{
        int ret;
        __asm__ volatile
        (
        "int $0x80;"
        : "=a" (ret)
        : "0"(SYS_open), "D"(buf), "S"(flags)
        : "cc", "rcx", "r11", "memory"
        );
        return ret;

}
