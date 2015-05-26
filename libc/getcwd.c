#include <stdlib.h>
#include <sys/syscall.h>

char* getcwd(char *buf, size_t size)
{
        char* ret;
        __asm__ volatile
        (
        "int $0x80;"
        : "=a"(ret)
        : "0"(SYS_getcwd), "D"(buf), "S"(size)
        : "cc", "rcx", "r11", "memory"
        );
        return ret;
}
