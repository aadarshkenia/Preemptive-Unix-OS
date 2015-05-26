//printf for printing time
#include <sys/sbunix.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

int yPosition = 23;
char pc[3];

void write_time(volatile char *outTh,volatile int *xPosition)
{
	unsigned short *mPos = (unsigned short *) 0xFFFFFFFF800B8000;

	while(*outTh)
	{
		mPos = (unsigned short *)(0xFFFFFFFF800B8000) + (*xPosition) + (yPosition * 80);
		(*xPosition)++;
		*mPos = (0x02 << 8 | *outTh++);
		
	}//end of while(*outTh)
}//end of write_time()

void print_time(const char *format, ...) {
	va_list val;

	volatile int xFixed = 65;

	int i;
	//char pc[3];
					
	va_start(val, format);
	while(*format) {
		if(*format == '%') {
			format++;
			switch(*format) {
				case 'd':
					i = va_arg(val, int);
					if(i<10)
					{
						pc[1]=(char)(48+i);
						pc[0]=(char)(48);//'0' for no second digit
					}
					else
					{	
						//Put two digits one by one into pc[] and call write_time(pc)
						pc[0]=(char)(48+i/10);
						pc[1]=(char)(48+i%10);
					}
					pc[3]='\0';
					write_time(pc,&xFixed);
					break;
				
				case '%':
				default:
					break;
			}
			++format;
		} else {
			char a[2];
			a[0]=*format;
			a[1] = '\0';
			format++;
			write_time(a,&xFixed);
		}
	}
	va_end(val);
}


