#include <string.h>

int strcmp(const char *firstString,const char *secondString){
    while(*firstString && *secondString){
        if(*firstString == *secondString){
            firstString++;
            secondString++;
            continue;
        }else
            return *firstString - *secondString;
    }
    return *firstString - *secondString;
}
