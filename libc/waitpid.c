#include <stdlib.h>
#include <sys/syscall.h>

pid_t waitpid(pid_t pid, int *status, int options)
{
    pid_t ret;
    __asm__ volatile
    (
    "int $0x80;"
    : "=a"(ret)
    : "0"(SYS_wait4), "D"(pid), "S"(status), "d"(options)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;	
}
