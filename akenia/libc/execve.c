#include <stdlib.h>
#include <sys/syscall.h>

int execve(const char *filename, char *const argv[], char *const envp[])
{
        int ret;
        __asm__ volatile
        (
        "syscall"
        : "=a"(ret)
        : "0"(SYS_execve), "D"(filename), "S"(argv), "d"(envp)
        : "cc", "rcx", "r11", "memory"
        );
        return ret;
}
