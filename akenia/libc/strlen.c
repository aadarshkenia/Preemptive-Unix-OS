#include <string.h>
#include <stdlib.h>

size_t strlen(const char *string){
    size_t count = 0;
    while(*string){
        string++;
        count++;
    }
    return count;
}
