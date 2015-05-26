#include <stdio.h>
#include <stdlib.h>

//int alpha = 1;

int main(int argc, char* argv[], char* envp[]) 
{	
	int pid;
	int ret;

	pid = fork();
	if(pid)
	{
		 // printf("Entering Shell\n");
	}
	else
	{
		
		char * a = "bin/shell";
		char *const nullchar = NULL;
		char *ar[2] = {a,nullchar};

		char *const e1 = "HOME=/Users/kavana";
		char *const e2 = "LIB=/lib";
		char *const e3 = "PATH=/bin";
		char * er[] = {e1,e2,e3,nullchar};                                                          
	        
	    //printf("%x %x %x %x\n",er,&er[0],&er[1],&er[2]);
		// printf("execvtest ar:%x er:%x\n",ar,er);

		ret = execve(a,ar,er);
		printf("%d\n",ret);
	}
	
	return 0;
}
