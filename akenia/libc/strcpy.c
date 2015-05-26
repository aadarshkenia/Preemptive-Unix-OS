#include <string.h>

char* strcpy(char *destinationString,const char *sourceString){
    while(*sourceString){
        *destinationString = *sourceString;
        sourceString++;
        destinationString++;
    }
    *destinationString='\0';
    return destinationString;
}
