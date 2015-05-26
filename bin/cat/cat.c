#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_FILE_SIZE 1024
//cat = open file + read file contents + print contents 

int main(int argc, char* argv[], char* envp[]) 
{
	int i = 1;
	char *buf = (char *)malloc(sizeof(char)*MAX_FILE_SIZE); 
	ssize_t readRet;
	char *input_string = (char *)malloc(sizeof(char)*512);
	// int cat_break=0;
	int filedesc;

	printf("argc[cat]:%d\n",argc);

	if (argc == 1)
	{	
		// while(1)
		// {
			scanf("%s",input_string);
			// for (cat_break=0;cat_break<strlen(input_string);cat_break++)
			// {
			// 	if (input_string[cat_break] =='!') exit(0);
			// }		
			printf("%s\n",input_string);
		// }
	}

	while (argc > 1)							
	{
		filedesc = open(argv[i], O_RDONLY);	
		if(filedesc != -1)
		{
			readRet = read(filedesc, (void *)buf,MAX_FILE_SIZE);
			if(readRet == 0)
				printf("no contents");
			else
				printf("%s\n",buf);
			argc--;
			i++;
		}
		else 
		{
			printf("cat: %s cannot be found\n",argv[i]);
			argc--;
			i++;
		}
	}

	free(buf);
	free(input_string);
	
	return 0;
}
