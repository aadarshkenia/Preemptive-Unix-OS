#include<sys/sbunix.h>
#include<stdlib.h>

void *memcpy(void * dest, void *src, size_t n)
{
    char *dp = dest;
    const char *sp = src;
    while (n--)
        *dp++ = *sp++;
    return dest;
}
