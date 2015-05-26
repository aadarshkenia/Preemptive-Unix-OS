#include <string.h>
#include <stdlib.h>

void* memset(void *string, int character, size_t length){
    char *str = string;
    while(length > 0){
        *str = character;
        str++;
        length--;
    }
    return string;
}
