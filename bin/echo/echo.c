#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[])
{

	int i=1,j;
	char *environ = (char *)malloc(sizeof(char)*512);
	int flag = 0;
	int length = 0;
	char *input_string = (char *)malloc(sizeof(char)*512);

	int count = argc ; //replace with argc later
	if(argc == 1){
		scanf("%s",input_string);
		printf("%s\n",input_string);
	}
	else
	{
		while (count>1)
		{
			if (argv[i][0] == '$') //envp here
			{
				strcpy(environ,&argv[i][1]);
				flag=0;
				length = strlen(environ)+1;
				for(j=0;envp[j];j++)
				{
	      		   	 if(strncmp(envp[j],environ,strlen(environ))==0)
					{
						printf("%s\n",envp[j]+length);
	            		flag=1;
						break;
	        		} 
				}
				if(flag==0)	
				{
	            		printf("%s is not a valid environment variable",environ);
						exit(0);
				}    
			}
			else
			{	
				printf ("%s\n",argv[i]);
			}
			i++;
			count--;
		}
	}
	free(environ);
	return 0;
}


