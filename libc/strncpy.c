#include <string.h>

char* strncpy(char *destinationString,const char *sourceString,int n){
    int count = 0;
    while(*sourceString && count < n){
        *destinationString = *sourceString;
        sourceString++;
        destinationString++;
        count++;
    }
    *destinationString='\0';
    return destinationString;
}
