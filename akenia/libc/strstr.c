#include <string.h>

char* strstr(char *string,char *substring){
    char *mainstr = string;
    if(*substring == '\0')
        return mainstr;
    while(*mainstr){
        char *mainptr = mainstr;
        char *subptr = substring;
        while(*mainptr && *subptr){
            if(*mainptr == *subptr){
                mainptr++;
                subptr++;
                continue;
            }else
                break;
        }
        if(*subptr == '\0')
            return mainstr;
        mainstr++;
    }
    return NULL;
}
