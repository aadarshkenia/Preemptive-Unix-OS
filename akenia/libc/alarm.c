#include <stdlib.h>
#include <sys/syscall.h>

unsigned int alarm(unsigned int seconds)
{
	unsigned int ret;
        __asm__ volatile
        (
        "syscall"
        : "=a"(ret)
        : "0"(SYS_alarm), "D"(seconds)
        : "cc", "rcx", "r11", "memory"
        );
	return ret;
}
