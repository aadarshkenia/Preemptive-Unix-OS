#include <stdlib.h>
#include <string.h> 
#include <stdio.h>

uint64_t atoi(char *str)
{
    uint64_t res = 0, sign = 1;
    if (str[0] == '-') {
        sign = -1;
        str++;
    } else if (str[0] == '+') {
        sign = 1;
        str++;
    }
    while (*str) {
        if ( (uint64_t)(*str) >= 48 && (uint64_t)(*str) <= 57) {
            res = res*10+(*str)-'0';
            str++;
        } else 
            return 0;
    }
    return res*sign;
}

int main(int argc, char *argv[], char *envp[])
{
    if(argc==3)
    {
    	if(!strcmp(argv[1],"-9"))
    	{
    		KillPCB(atoi(argv[2]));
    	}
    }
    return 0;
}