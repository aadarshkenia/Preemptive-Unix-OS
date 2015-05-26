#include <stdlib.h>
#include <sys/syscall.h>

struct timespec_users
{
    signed long sec;
    long nsec;
};

unsigned int sleep(unsigned int seconds)
{
       	unsigned int ret;
        struct timespec_users sleep_time = {seconds,0};
        struct timespec_users rem_time = {0};
        struct timespec_users *rqtp,*rmtp;
        rqtp = &sleep_time;
        rmtp=&rem_time;
        __asm__ volatile
        (
        "syscall"
        : "=a"(ret)
        : "0"(SYS_nanosleep), "D"(rqtp),"S"(rmtp)
        : "cc", "rcx", "r11", "memory"
        );
	return ret;
}
