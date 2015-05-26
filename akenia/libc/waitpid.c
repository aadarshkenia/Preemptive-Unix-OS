#include <stdlib.h>
#include <sys/syscall.h>

pid_t waitpid(pid_t pid, int *status, int options)
{
    pid_t ret;
    __asm__ volatile
    (
    "syscall"
    : "=a"(ret)
    : "0"(SYS_wait4), "D"(pid), "S"(status), "d"(options)
    : "cc", "rcx", "r11", "memory"
    );
    return ret;	
}
