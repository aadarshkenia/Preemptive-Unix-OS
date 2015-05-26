#include <sys/kbd.h>
#include <sys/defs.h>
#include <sys/ports_io.h>
#include <sys/sbunix.h>
#include <string.h>
#include <sys/page.h>
#include <sys/structs.h>

extern uint64_t *kernel_pml4;

#define ESC 27
#define BACKSPACE '\b'
#define TAB '\t'
#define ENTER '\n'
#define RETURN '\r'
#define NEWLINE ENTER
#define KF1 0
#define KF2 0
#define KF3 0
#define KF4 0
#define KF5 0
#define KF6 0
#define KF7 0
#define KF8 0
#define KF9 0
#define KF10 0
#define KF11 0
#define KF12 0
#define KHOME 0
#define KUP 0
#define KPGUP 0
#define KLEFT 0
#define KRIGHT 0
#define KEND 0
#define KDOWN 0
#define KPGDN 0
#define KINS 0
#define KDEL 0
#define KRLEFT_SHIFT 0x2A
#define KRRIGHT_SHIFT 0x36
#define KCTRL 0x1D
#define KEY_RELEASED(s) (s&0x80)
#define SHIFT_CODE(x) (x == KRLEFT_SHIFT || x == KRRIGHT_SHIFT)
#define CTRL_CODE(x) (x == KCTRL)

int shift_down;
int ctrl_down;

extern char *readBuff;
extern void *addressOfBuff;
extern struct PCB *pcb_head;
extern struct PCB *block_pcb_head;
extern struct PCB *current_pcb;

static char asciiNonShift[] = {
0 , ESC, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', BACKSPACE,
TAB, 'q', 'w',   'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',   '[', ']', ENTER, 0,
'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, 0, 0, ' ', 0,
KF1, KF2, KF3, KF4, KF5, KF6, KF7, KF8, KF9, KF10, 0, 0,
KHOME, KUP, KPGUP,'-', KLEFT, '5', KRIGHT, '+', KEND, KDOWN, KPGDN, KINS, KDEL, 0, 0, 0, KF11, KF12 };


static char asciiShift[] = {
0, ESC, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', BACKSPACE,
TAB, 'Q', 'W',   'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P',   '{', '}', ENTER, 0,
'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~', 0, '|',
'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0, 0, 0, ' ', 0,
KF1,   KF2, KF3, KF4, KF5, KF6, KF7, KF8, KF9, KF10, 0, 0,
KHOME, KUP, KPGUP, '-', KLEFT, '5',   KRIGHT, '+', KEND, KDOWN, KPGDN, KINS, KDEL, 0, 0, 0, KF11, KF12 };

static char scancode_to_ascii(int scancode)
{
	if(shift_down){
		shift_down = 0;
		return asciiShift[scancode];
	}
	else
		return asciiNonShift[scancode];
}

void init_keyboard()
{
	shift_down = 0;
	ctrl_down = 0;
// 	outb(0x21,0xfd);			//For PIC 1 i think
// 	outb(0xa1,0xff);			//Similarly, for PIC2
}

void keyboard_handler(struct regs r)
{
	char scanned;
	char *temp = readBuff;
	struct PCB *temp1 = pcb_head;
	unsigned char scancode;
	//unsigned char str[2];
	//str[0]='\0';str[1]='\0';
	unsigned char str[3];	
	str[0]='\0';str[1]='\0';str[2]='\0';

	scancode = inb(0x60);
	
	if (KEY_RELEASED(scancode)) {
		scancode &= 0x7F;  // WHY THIS LINE
		if(SHIFT_CODE(scancode)) shift_down=0;
// 		return; 	// IF UNCOMMENTED WILL PRINT CHARACTER ONLY ONCE
	} else {
		if (SHIFT_CODE(scancode)) {
			shift_down=1;
		} else if (CTRL_CODE(scancode)) {
			ctrl_down = 1;
		} else {
			if(ctrl_down) {
				str[0]='^';
				scanned = scancode_to_ascii(scancode);
				str[1]=scanned;
				ctrl_down=0;
			} else {
				scanned = scancode_to_ascii(scancode);
				str[0]=scanned;
				str[1]=0;
				printf("%s",str);	
				
				if (scanned != '\n') {
					while (*readBuff) readBuff++;
					*readBuff++ = scanned;
					*readBuff = '\0';
					readBuff = temp;
				} else {
					if (block_pcb_head) {
						loadcr3(block_pcb_head -> cr3);
						readBuff[strlen(readBuff)] = '\0';
						//readBuff[strlen(readBuff) + 1] = '\0';
						strcpy((char *)addressOfBuff, (char *)readBuff);
						readBuff = temp;

						memset(readBuff, 0, strlen(readBuff));
						readBuff = temp;
						//loadcr3((void *)PADDR(kernel_pml4));
						loadcr3(current_pcb -> cr3);
						//NEW 9th MAY
						block_pcb_head->isFore=1;
						while (temp1 -> next) temp1 = temp1 -> next;
						temp1 -> next = block_pcb_head;
						block_pcb_head -> prev = temp1;
						block_pcb_head -> next = NULL;
						block_pcb_head = NULL;
					} else {
						while (*readBuff) readBuff++;
						*readBuff++ = scanned;
						*readBuff = '\0';
						readBuff = temp;
					}
				}
			}
		}
	}
	outb(0x20,0x20);
}


// #include <sys/kbd.h>
// #include <sys/defs.h>
// #include <sys/ports_io.h>
// #include <sys/sbunix.h>
// 
// // unsigned char keyboardLower[128] = "\0\e1234567890-=\b\tqwertyuiop[]\n\0asdfghjkl;\'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0\0+\0\0\0\0\0\0\0\0\0\0\0";
// // unsigned char keyboardUpper[128] = "\0\e!@#$%^&*()-=\b\tQWERTYUIOP{}\n\0ASDFGHJKL:\"~\0|ZXCVBNM<>?\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0\0+\0\0\0\0\0\0\0\0\0\0\0";
// // int shift = 0;
// // int ctrl = 0;
// 
// void init_keyboard(){
// 	// shift = 0;
// // 	ctrl = 0;
// }
// 
// void keyboard_handler(struct regs r){
// 	shift = 0;
// 	ctrl = 0;
// 	unsigned char scancode;
// 	scancode = inb(0x60);
// 
// 	if(scancode & 0x80){
// 
// 	}else{
// 		if(scancode == 42){ // 42 - Check if left shift is pressed
// 			shift = 1;
// 		}else if (scancode == 54){	// 42 - Check if right shift is pressed
// 			shift = 1;
// 		}else if(scancode == 29){	// 29 - Check if ctrl is pressed
// 			ctrl = 1;
// 		}else  if(scancode == 5){  //  5 - backspace
// 			printf("%c",keyboardLower[scancode]);
// 		}else if(scancode == 28){	// 28 - enter or return
// 			printf("%c",keyboardLower[scancode]);
// 		}else{
// 			if(shift == 1){
// 				printf("%c",keyboardUpper[scancode]);
// 			}else if(ctrl == 1){
// 				printf("^");
// 				printf("%c",keyboardLower[scancode]);
// 			}else{
// 				printf("%c",keyboardLower[scancode]);
// 			}
// 		}
// 
// 	}
// outb(0x20,0x20);
// }
