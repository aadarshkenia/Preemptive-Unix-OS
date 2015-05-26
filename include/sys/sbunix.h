#ifndef _SBUNIX_H
#define _SBUNIX_H

#include <sys/defs.h>
#include <stdlib.h>
void printf(const char *format, ...);
void print_time(const char *format, ...);	
void print_glyph(const char *format, ...);
void * memcpy (void *dest, void * src, size_t n);
int puts(const char *str);

#endif
