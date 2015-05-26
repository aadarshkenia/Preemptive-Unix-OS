#include <sys/sbunix.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

int yPositionGlyph = 23;
char *pc_glyph = NULL;

void write_glyph(volatile char *outTh,volatile int *xPosition)
{
	unsigned short *mPos = (unsigned short *) 0xFFFFFFFF800B8000;

	while(*outTh)
	{
		mPos = (unsigned short *)(0xFFFFFFFF800B8000) + (*xPosition) + (yPositionGlyph * 80);
		(*xPosition)++;
		*mPos = (0x02 << 8 | *outTh++);
		
	}//end of while(*outTh)
}//end of write_glyph()

void print_glyph(const char *format, ...) {
	va_list val;

	volatile int xFixed = 75;
					
	va_start(val, format);
	while(*format) {
		if(*format == '%') {
			format++;
			switch(*format) {
			
				case 's':
					pc_glyph = va_arg(val,char *);
					write_glyph(pc_glyph,&xFixed);
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
			write_glyph(a,&xFixed);
		}
	}
	va_end(val);
}


