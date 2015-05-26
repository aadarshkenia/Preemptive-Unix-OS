#include <string.h>
#include <stdlib.h>

int strncmp(const char *firstString,const char *secondString,size_t n){
    int count=1;
    while(*firstString && *secondString && count < n){
        if(*firstString == *secondString){
            firstString++;
            secondString++;
            count++;
            continue;
        }else
            return *firstString - *secondString;
    }
    return *firstString - *secondString;
}
