#include <sys/sbunix.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define MAXIMUM_SIZE 512
volatile int xPos = 0;
volatile int yPos = 0;
unsigned short *memPos = (unsigned short *) 0xFFFFFFFF800B8000;
unsigned short *memPosSc = (unsigned short *) 0xFFFFFFFF800B8000;

char ptr[MAXIMUM_SIZE];
char *convert(uint64_t inputNumber, int base) {
	int outputNumber[MAXIMUM_SIZE], i = 0, j = 0;
	if (!inputNumber) return "0";
	while (inputNumber != 0) {
		outputNumber[j++] = inputNumber % base;
		inputNumber = inputNumber / base;
	}
	for(--j; j>=0; j--) ptr[i++] = "0123456789ABCDEF"[outputNumber[j]];
	ptr[i] = '\0';
	return &ptr[0];
}

void writek(volatile char *outTh)
{
	while(*outTh)
	{
		if(*outTh == 10){ //newline
			yPos++;
			xPos = 0;
			outTh++;
		}
		else if(*outTh == 13){ //carriage return
			xPos = 0;
			outTh++;
		}else{
			if(xPos >= 80){
				yPos++;
				xPos = 0;
			}
			if (yPos > 24){
				memPos = (unsigned short *) 0xFFFFFFFF800B8000;
				memPosSc = (unsigned short *) 0xFFFFFFFF800B8000 + 80;
				memcpy(memPos, memPosSc, (size_t) 3840);
				memPosSc = (unsigned short *) 0xFFFFFFFF800B8000 + 1920;
				memset(memPosSc, 0, 160);
				yPos = 24;
				xPos = 0;

			}//end of if y>24
			
			memPos = (unsigned short *)(0xFFFFFFFF800B8000) + (xPos) + (yPos * 80);
			xPos++;
			*memPos = (0x02 << 8 | *outTh++);
		}
	}//end of while(*outTh)
}//end of writek()

void printf(const char *format, ...) {
	va_list val;
	char *s  = NULL;
	int i;int hp = 1;char pc[2];
	va_start(val, format);
	while(*format) {
		if(*format == '%') {
			format++;
			switch(*format) {
				case 'c':
					*s = va_arg(val, int);
					*(s + 1) = '\0';
					writek(s);
					break;
				case 'd':
					i = va_arg(val, int);
					if(i==0){
					writek("0");
					writek("\0");
					}
					else
					{	//Negative nos
						if(i < 0) {
							i = -1 * i;
							writek("-");
						}
						//Put digits one by one into s and call writek(s)
						while(i>=hp)
						{
							hp=hp*10;
						}
						hp=hp/10;
						while(hp>0)
						{
							pc[0]=(char)(48+i/hp);
							pc[1]='\0';
							writek(pc);
							i=i%hp;
							hp=hp/10;	
						}				
						hp=1;
					}
					break;
				case 'o':
					s = convert(va_arg(val, int), 8);
					writek(s);
					break;
				case 's':
					s = va_arg(val,char *);
					writek(s);
					break;
				case 'u':
					s = convert(va_arg(val, uint64_t), 10);
					writek(s);
					break;
				case 'x':
					s = convert(va_arg(val, uint64_t), 16);
					writek("0x");
					writek(s);
					break;
				case 'p':
					s = convert(va_arg(val, uint64_t), 16);
					writek(s);
					break;
				case '%':
				default:
					break;
			}
			++format;
		} else {
			char a[2];
			a[0] = *format;
			a[1] = '\0';
			format++;
			writek(a);
		}
	}
	va_end(val);
}




/*#include <sys/sbunix.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#define MAXIMUM_SIZE 512
//Changed to static here
volatile int xPos = 0;
volatile int yPos = 0;
unsigned short *memPos = (unsigned short *) 0xFFFFFFFF800B8000;
unsigned short *memPosSc = (unsigned short *) 0xFFFFFFFF800B8000;

char ptr[MAXIMUM_SIZE];
char *convert(unsigned int inputNumber, int base) {
	int outputNumber[MAXIMUM_SIZE], i = 0, j = 0;
	
	if(!inputNumber)
		return "0";
	while (inputNumber != 0) {
		outputNumber[j++] = inputNumber % base;
		inputNumber = inputNumber / base;
	}
	for(--j; j>=0; j--) ptr[i++] = "0123456789ABCDEF"[outputNumber[j]];
	ptr[i] = '\0';
	return &ptr[0];
}

void writek(volatile char *outTh)
{
	//static int xPos=0;
	//static int yPos=0;
	while(*outTh)
	{
		if(*outTh == 10){ //newline
			yPos++;
			xPos = 0;
			outTh++;
		}
		else if(*outTh == 13){ //carriage return
			xPos = 0;
			outTh++;
		}else{
			if(xPos >= 80){
				yPos++;
				xPos = 0;
			}
			if (yPos > 24){
				memPos = (unsigned short *) 0xFFFFFFFF800B8000;
				memPosSc = (unsigned short *) 0xFFFFFFFF800B8000 + 80;
				memcpy(memPos, memPosSc, (size_t) 3840);
				memPosSc = (unsigned short *) 0xFFFFFFFF800B8000 + 1920;
				//for(;memPosSc < (unsigned short *) 0xB8000 + 4000; memPosSc++){}
				memset(memPosSc, 0, 160);
				yPos = 24;
				xPos = 0;

			}//end of if y>24
			
			memPos = (unsigned short *)(0xFFFFFFFF800B8000) + (xPos) + (yPos * 80);
			xPos++;
			*memPos = (0x02 << 8 | *outTh++);
		}
	}//end of while(*outTh)
}//end of writek()

void printf(const char *format, ...) {
	va_list val;
	//char * s = "        ";
	char *s  = NULL;
	int i;int hp=1;char pc[2];
	va_start(val, format);
	while(*format) {
		if(*format == '%') {
			format++;
			switch(*format) {
				case 'c':
					*s = va_arg(val, int);
					*(s + 1) = '\0';
					writek(s);
					//printed++;
					break;
				case 'd':
					i = va_arg(val, int);
					if(i==0){
					writek("0");
					writek("\0");
					}
					else
					{	//Negative nos
						if(i < 0) {
							i = -1 * i;
							writek("-");
							//printed++;
						}
						//Put digits one by one into s and call writek(s)
						while(i>=hp)
						{
							hp=hp*10;
						}
						hp=hp/10;
						while(hp>0)
						{
							pc[0]=(char)(48+i/hp);
							pc[1]='\0';
							writek(pc);
							i=i%hp;
							hp=hp/10;	
						}				
						hp=1;
					}
					break;
				case 'o':
					s = convert(va_arg(val, int), 8);
					writek(s);
					break;
				case 's':
					s = va_arg(val,char *);
					writek(s);
					break;
				case 'u':
					s = convert(va_arg(val, unsigned int), 10);
					writek(s);
					break;
				case 'x':
					s = convert(va_arg(val, unsigned int), 16);
					writek("0x");
					writek(s);
					break;
				case 'p':
					s = convert(va_arg(val, unsigned int), 16);
					writek(s);
					break;
				case '%':
				default:
					break;
			}
			++format;
		} else {
			//char *a = " ";
			char a[2];
			a[0]=*format;
			//strcpy(a, format++);
			a[1] = '\0';
			format++;
			//writek((char *)a);
			writek(a);
		}
	}
	va_end(val);
	// writek("1234567890qwertyuiopasdfghjklzxcvbnmQWER\nFGHJKLZXCVBNM!@#$^&*()");
//	free(s);
}

*/
