#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[], char* envp[]) {


	char *buff = (char *)malloc(128);			
	char *cwd= (char *)malloc(128);
	void *dir;
	struct dirent *dp;

	// printf("argc:%d\n",argc);
	// for(int i=0;i<argc;i++)
	// 	printf("argv[%d]:%s\n",i,argv[i]);

	if (argc==1){							
		strcpy(cwd,getcwd(buff,128));
		if(cwd == NULL){
			printf("Error\n");
			return 0;
		}
	}
	else{ 
		strcpy(cwd,argv[1]);				// test when argc is set up
	}
	//printf("cwd: %s\n",cwd);
	dir = (void *)opendir(cwd);

	if(dir != NULL){
		while((dp = readdir((void *)dir)) != NULL)
		{
			printf("%s ",dp->d_name);
		}
		printf("\n");
	}
	else
		printf ("Directory not found\n") ;

	free(cwd);
	free(buff);
	return 0;
}

//ls for testing of &

/*
int main(int argc, char* argv[], char* envp[]) {
	int i=0;
	for(i=0;i<1000;i++){
		printf("ls\n");
	}
}
*/