#include <stdio.h>
#include <stdlib.h>

//int alpha = 1;

int main(int argc, char* argv[], char* envp[]) 
{
	int pid=0;
	int *status = malloc(sizeof(int));
	

	char *input1 = (char *)malloc(20);
	char *input2 = (char *)malloc(20);

	pid = fork();
	if(pid)
	{
		
		printf("Parent: Enter something\n");
		waitpid(pid, status,0);

		scanf("%s",input1);
		printf("Parent entered: %s\n",input1);
	}
	else
	{
		
		printf("Child: Enter something: \n");
		scanf("%s",input2);
		printf("Child entered: %s\n",input2);
	}
	return 0;
}
