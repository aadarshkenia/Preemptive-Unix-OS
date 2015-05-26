#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

int scanf(const char *format, ...) {
    va_list val;
    int printed = 0;
    char *i = (char *)malloc(256);
    ssize_t readlen =0;
    int *k;
    int c = 0, sign = 1, n = 0;
    ssize_t j = 0;
    va_start(val, format);
    while(*format) {
        if(*format == '%') {
            format++;
            switch(*format) {
                case 'c':
                    i = va_arg(val, char *);
                    printed += read(0, i, 1);
                    break;
                case 'd':
                    k = va_arg(val, int *);
                    readlen = read(0, i, 7);
                    printed+=readlen;
                    i[readlen - 1] = '\0';
                    if (i[0] == '-') {
                        sign = -1;
                        c = 1;
                    }
                    while(c<(readlen-1))
                    {
                        printf(" i of c is %c ",i[c]);
                        n = (n * 10) + (i[c] - '0');
                        c++;
                    }
                    n *= sign;
                    *k = n;
                    printed--;
                    break;
                case 's':
                    i = va_arg(val, char *);
                    j = read(0, i, 256);
                    i[j] = '\0';
                    printed += j;
                    printed--;
                    break;
                case 'x':
                    k = va_arg(val, int *);
                    readlen = read(0, i, 7);
                    printed+=readlen;
                    i[readlen - 1] = '\0';
                    while(c<(readlen-1))
                    {
                        printf(" i of c is %c ",i[c]);
                        if( i[c]>= 'A' && i[c] <= 'F')
                            n = (n * 16) + i[c]-'A'+ 10;
                        else
                            n = (n * 16) + (i[c] - '0');
                        c++;
                    }
                    *k = n;
                    printed--;
                    break;
                case 'o':
                    k = va_arg(val, int *);
                    readlen = read(0, i, 7);
                    printed+=readlen;
                    i[readlen - 1] = '\0';
                    while(c<(readlen-1))
                    {
                        if(i[c]<'9' && i[c]>'0')
                            n = (n * 8) + (i[c] - '0');
                        c++;
                    }
                    *k = n;
                    printed--;
                    break;
                case '%':
                default:
                    break;
            }
            ++format;
        } //else printed += read(0, format++, 1);
    }
    va_end(val);
    return printed;
}
