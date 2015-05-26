#ifndef _STRING_H_
#define _STRING_H_

#include <stdlib.h>

int strcmp(const char *,const char *);
int strncmp(const char *,const char *,size_t);
char* strcpy(char *,const char *);
size_t   strlen(const char *);
char* strstr(char *,char *);
void* memset ( void *, int, size_t);
char* strncpy(char *destinationString,const char *sourceString,int n);
#endif
