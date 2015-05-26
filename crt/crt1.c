#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[], char* envp[]);


uint64_t * rsp;
char **argv=NULL;
char **envp=NULL;
int argc;


void _start(int stacktop) 
{
	
	int i=1;
	int count=0;
	__asm__ __volatile__ ("movq %%rsp, %0;" : "=r"(rsp));
	argc = *(rsp+1);
	//printf("argc val: %d\n",argc);
	//printf("i val: %d\n",i);
	//printf("count val: %d\n",count);

	
	if(argc)
	{
		rsp=rsp+2;
		argv = (char **)rsp;
		for(i=0;i<argc;i++)
		{
			argv[i] = (char *)(*rsp);
			rsp++; 
		}
	
		//printf("%s\n",argv[0]);
		rsp++; // For null
		
		while(*(rsp+i))
		{
			count++;
			i++;
		}
		

		envp = (char **)rsp;
		for(i=0;i<count;i++)
		{
			envp[i] = (char *)(*rsp);
			rsp++;
		}

		//printf("%s\n",envp[0]);
		main(argc,argv, envp);
		
	}
	else
	{
		main(argc,NULL,NULL);
	}

	exit(0);

} 

// old
// #include <stdlib.h>
// #include <stdio.h>

// int main(int argc, char* argv[], char* envp[]);


// uint64_t * rsp;
// char **argv=NULL;
// char **envp=NULL;
// int argc;


// void _start(int stacktop) 
// {
	
// 	int i=1;
// 	int count=0;
// 	__asm__ __volatile__ ("movq %%rsp, %0;" : "=r"(rsp));
// 	argc = *(rsp+1);
// 	//printf("argc val: %d\n",argc);
// 	//printf("i val: %d\n",i);
// 	//printf("count val: %d\n",count);

	
// 	if(argc)
// 	{
		
// 		argv = malloc(argc*sizeof(char *));
// 		for(i=0;i<argc;i++)
// 		{
// 			argv[i] = (char *)*(rsp+2+i); 
// 		}
	
// 		//printf("%s\n",argv[0]);
		
// 		i=0;
// 		rsp = rsp+3+argc;
// 		while(*(rsp+i))
// 		{
// 			count++;
// 			i++;
// 		}
		

// 		envp = malloc(count*sizeof(char *));
// 		for(i=0;i<count;i++)
// 		{
// 			envp[i] = (char *)*(rsp+i);
// 		}

// 		//printf("%s\n",envp[0]);
// 		main(argc,argv, envp);
		
// 	}
// 	else
// 	{
// 		//envp starts directly if 0 argc



// 		main(argc,NULL,NULL);
// 	}

// 	exit(0);

// }
