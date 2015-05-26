/*#include <stdlib.h>
#include <stdarg.h>

int printf(const char *format, ...) {
        va_list val;
        int printed = 0;

        va_start(val, format);

        while(*format) {
                write(1, format, 1);
                ++printed;
                ++format;
        }

        return printed;
}*/





#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define MAXIMUM_SIZE 512

char ptr[MAXIMUM_SIZE];

char *convert(unsigned int inputNumber, int base) {
    int outputNumber[MAXIMUM_SIZE], i = 0, j = 0;
    while (inputNumber != 0) {
        outputNumber[j++] = inputNumber % base;
        inputNumber = inputNumber / base;
    }
    for(--j; j>=0; j--) ptr[i++] = "0123456789ABCDEF"[outputNumber[j]];
    ptr[i] = '\0';
    return &ptr[0];
}

int printf(const char *format, ...) {
brk(0);
    va_list val;
    int printed = 0;
 char *s = malloc(MAXIMUM_SIZE);
// char s[100];
 int i;
    unsigned int j=0;
    int temp = 0;
    long divi = 10;
    va_start(val, format);
    while(*format) {
        if(*format == '%') {
            format++;
            switch(*format) {
                case 'c':
                    *s = va_arg(val, int);
                    *(s + 1) = '\0';
                    write(1, s, 1);
                    printed++;
                    break;
                case 'd':
                    i = va_arg(val, int);
		    if(i==0){
			printed+=write(1,"0\0",2);
			}else{
                    if(i < 0) {
                        i = -1 * i;
                        write(1, "-", 1);
                        printed++;
                    }
                    while(i / divi) divi = divi * 10;
                    divi = divi / 10;
                    while(i > 0) {
                        temp = i / divi;
                        *(s + j) = (char) temp + 48;
                        j++;
                        i = i % divi;
                        divi = divi / 10;
                    }
                    *(s + j) = '\0';
                    printed += write(1, s, 2);
			}
                    break;
                case 'o':
                    s = convert(va_arg(val, int), 8);
                    printed += write(1, s, 2);
                    break;
                case 's':
                    s = va_arg(val,char *);
                    printed += write(1, s, strlen(s));
                    break;
                case 'u':
                    s = convert(va_arg(val, unsigned int), 10);
                    printed += write(1, s, 2);
                    break;
                case 'x':
                    s = convert(va_arg(val, unsigned int), 16);
                    printed += write(1, s, 2);
                    break;
                case '%':
                default:
                    break;
            }
            ++format;
        } else
            printed += write(1, format++, 1);
    }
    va_end(val);   s = NULL;
    free(s);
    return printed;
}

