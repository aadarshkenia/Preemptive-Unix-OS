#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NO_OF_COMMANDS 10
#define NO_OF_ARGUMENTS 128
#define COMMAND_ARGUMENT_LENGTH 128
#define NO_OF_PATHS 128
#define PS1PROMPT_LENGTH 64
#define COMMAND_STRING_LENGTH 512

void mystrcat(char *firstString,char *secondString){
    while(*firstString)
        firstString++;

    while(*secondString){
        *firstString=*secondString;
        secondString++;
        firstString++;
    }
    *firstString='\0';
}

void setPath(char *path, char* envp[]){
    int i=0;
    char *currentPath=NULL;
    char *checkForPATH;
    char *afterPATH;
    char *beforePATH;

    
    for(i=0;envp[i];i++){
        if(strncmp(envp[i],"PATH",4)==0){ 
            currentPath = malloc(strlen(envp[i])+1);
            strcpy(currentPath,envp[i]+5);
            break;
        }else
            continue;
    }
    // printf("old path: %s\n",envp[i]);
    // mystrcat(envp[i],":");
    // mystrcat(envp[i],path+5);

    checkForPATH = strstr(path,"{$PATH}");
    if(checkForPATH != NULL){
        afterPATH = malloc(strlen(checkForPATH)-6);
        strcpy(afterPATH,checkForPATH+7);
        while(*checkForPATH){
            *checkForPATH = '\0';
            checkForPATH++;
        }
        beforePATH = malloc(strlen(path)+1);
        strcpy(beforePATH, path);

        if(*beforePATH){
            strcpy(envp[i],beforePATH);
            mystrcat(envp[i],currentPath);
        }else
        strcpy(envp[i],currentPath);
        if(*afterPATH)
            mystrcat(envp[i],afterPATH);
            // printf("after string:%s-\n",afterPATH);
            // printf("before string:%s-\n",beforePATH);
            free(afterPATH);
            free(beforePATH);
        }else
            strcpy(envp[i],path);

    // if(currentPath != NULL)
    //     free(currentPath);
    // printf("new path:%s\n",envp[i]);
}

int getCommands(char *string,char **commands){
    int i=0,j=0,k=0,count=0,whitespaceCount=0;
    size_t len;

    commands[0] = malloc(sizeof(char)*NO_OF_ARGUMENTS);
    for(i=0;string[i]!='\0';i++){
        if(string[i]!='|'){
            commands[j][k] = string[i];
            k++;
        }else{
            commands[j][k]='\0';
            j++;
            commands[j] = malloc(sizeof(char)*NO_OF_ARGUMENTS);
            k=0;
        }
    }
    commands[j][k]='\0';

    for (i=0; commands[i]!=NULL; i++) {
        count++;
        len = strlen(commands[i]);
        j=0;
        whitespaceCount = 0;
        while (commands[i][j]== ' ') {
            whitespaceCount++;
            j++;
        }
        for (j=0; j<len; j++) {
            commands[i][j] = commands[i][j+whitespaceCount];
        }
        len = strlen(commands[i]);
        j=(int)len-1;
        while (commands[i][j] == ' ') {
            commands[i][j]='\0';
            j--;
        }
    }
    return count - 1;
}

void getArguments(char *string,char **args){

    int i=0,j=0,k=0;

    args[0] = malloc(sizeof(char)*NO_OF_ARGUMENTS);
    for(i=0;string[i]!='\0';i++){
        if(string[i]!=' '){
            args[j][k] = string[i];
            k++;
        }else{
            if(args[j][k-1]!='\0'){
            args[j][k]='\0';
            j++;
            args[j] = malloc(sizeof(char)*NO_OF_ARGUMENTS);
            k=0;
        }else
            continue;
    }
  }
  args[j][k]='\0';
}

char** getPathsAppendedWithCommand(char *command, char *envp[]){
    int i,j=0,k=0;
    char *pathvar=NULL;
    char **paths=NULL;

    for(i=0;envp[i];i++){
        if(strncmp(envp[i],"PATH",4)==0){
        	pathvar = malloc(strlen(envp[i])-4);
            strcpy(pathvar,envp[i]+5);
            break;
        }else
            continue;
    }
    printf("path: %s\n", pathvar);

    paths = malloc(sizeof(char *)* NO_OF_PATHS);
	for (i = 0; i < NO_OF_PATHS; i++){
		paths[i]= NULL;
	}

	paths[0]=malloc(sizeof(char)*NO_OF_ARGUMENTS);
    for(i=0;pathvar[i]!='\0';i++){
        if(pathvar[i]!=':'){
            paths[j][k] = pathvar[i];
            k++;
        }else{
            paths[j][k]='\0';
            j++;
            paths[j]=malloc(sizeof(char)*NO_OF_ARGUMENTS);
            k=0;
        }
    }
    paths[j][k]='\0';

    for(i=0;paths[i];i++){
        mystrcat(paths[i],"/");
        mystrcat(paths[i],command);
    }

 	return paths;
}

int execute(char **commands,char* envp[]){
	char **arguments = NULL;
	int i=0,q=0,status=0,retval=0;
	pid_t pid,pid1,pid_parent,pid1_parent;
	char **paths;

	arguments = malloc(sizeof(char *)* NO_OF_ARGUMENTS);
	for (i = 0; i < NO_OF_ARGUMENTS; i++){
		arguments[i]= NULL;
	}

	getArguments(commands[0],arguments);
	for (i = 0; arguments[i]; i++){
       	printf("arg[%d]:%s\n",i,arguments[i]);
    }

	pid = fork();

	if(pid == 0){
		printf("Child Process\n");
		q=execve(arguments[0],arguments,envp);
        printf("q value:%d\n",q);
		if(q < 0){
			paths = getPathsAppendedWithCommand(arguments[0],envp);
			pid1 = fork();
			if(pid1 == 0){
				printf("Child Process 1\n");
				for(i=0;paths[i];i++){
      		    // printf("%d - %s\n",i,paths[i]);
      		    strcpy(arguments[0],paths[i]);
      		    q=execve(arguments[0],arguments,envp);
      		    if(q < 0){
      			   continue;
      		    }
   			}
   			printf("Invalid Command\n");
   			exit(1);
			}else if(pid1 == -1){
				printf("fork failed\n");
			}else{
				printf("Parent Process 1\n");
				pid1_parent = waitpid(-1,&status,0);
				printf("status: %d, pid: %d\n",status,pid1_parent);
                for (i = 0;paths[i]; i++){
                    free(paths[i]);
                }
                free(paths);
                if(status!=0){
                    printf("ERROR\n");
                    exit(1);
                }else
                    exit(0);
            }
	   }
	}else if(pid == -1){
		printf("fork failed\n");
	}else{
		printf("Parent Process\n");
		pid_parent = waitpid(-1,&status,0);
		printf("pid: %d\n",pid_parent);
		printf("status: %d \n",status);
        if(status!=0)
            retval = -1;
	}
    return retval;
}

int piping(int pipesCount,char **str[],char* envp[])
{
    int index=0,status=0,q=0,pipeIndex=0,i=0,retval=0;
    char **paths;
    pid_t pid,pid_parent,pid1,pid1_parent;
    int pipesfd[pipesCount * 2];
    for(i=0; i<pipesCount;i++){
        if(pipe(pipesfd+(i*2))==-1){
            printf("pipe failed\n");
            exit(1);
        }
    }

    while(str[index]!=NULL){
        printf("%d\n",index);
        pid = fork();
        if(pid==0){
            printf("Child Process\n");
            if(pipeIndex!=0){
                if(pipesfd[pipeIndex-2]!=0){
                    printf("Input from pipe\n");
                    if(dup2(pipesfd[pipeIndex-2],0)==-1){
                        printf("dup2 failed 1\n");
                        exit(1);
                    }
                }
            }
            if(pipesCount > 0 && index<pipesCount){
                if(pipesfd[pipeIndex+1]!=1){
                    printf("Output to pipe\n");
                    if(dup2(pipesfd[pipeIndex+1],1)==-1){
                        printf("dup2 failed 2\n");
                    }
                }
            }
            if(pipeIndex != 0){
                close(pipesfd[pipeIndex-2]);
            }
            q=execve(str[index][0],str[index],envp);
            printf("q value:%d\n",q);
            if(q==-1){
                paths = getPathsAppendedWithCommand(str[index][0],envp);
                pid1 = fork();
                if(pid1 == 0){
                printf("Child Process 1\n");
                for(i=0;paths[i];i++){
                    // printf("%d - %s\n",i,paths[i]);
                    strcpy(str[index][0],paths[i]);
                    q=execve(str[index][0],str[index],envp);
                    if(q==-1){
                        continue;
                    }
                }
                printf("Invalid Command\n");
                exit(1);
                }else if(pid1 == -1){
                    printf("fork failed\n");
                }else{
                    printf("Parent Process 1\n");
                    pid1_parent = waitpid(-1,&status,0);
                    printf("status: %d, pid: %d\n",status,pid1_parent);
                    for(i = 0;paths[i]; i++){
                        free(paths[i]);
                    }
                    free(paths);
                    if(status!=0){
                        printf("ERROR\n");
                        exit(1);
                    }else
                        exit(0);
                }
            } 
        }else if(pid == -1){
            printf("fork failed\n");
        }else{
            printf("Parent Process\n");
            pid_parent = waitpid(-1,&status,0);
            printf("status: %d, pid: %d\n",status,pid_parent);
            if(status!=0){
                retval = -1;
                break;
            }else{
                if(index < pipesCount && pipesCount > 0){
                    close(pipesfd[pipeIndex+1]);
                }
                pipeIndex = pipeIndex + 2;
                index = index + 1;
            }
        }
    } // while end

    for(i=0;i<pipesCount*2;i++){
        close(pipesfd[i]);
    }
    return retval;
}


int main(int argc, char* argv[], char* envp[]){
	char *str=NULL;
    char *ps1Prompt = NULL;
    char *pathvar = NULL;
    char **commands = NULL;
    char ***allCommands=NULL;
    char *path = NULL;
    int noOfPipes = 0,returnres=0;
    int i=0,j=0,k=0;

    ps1Prompt = (char *)malloc(sizeof(char) * PS1PROMPT_LENGTH);
    memset(ps1Prompt, 0, PS1PROMPT_LENGTH);
    strcpy(ps1Prompt, "sbush");

    str = (char *)malloc(sizeof(char) * COMMAND_STRING_LENGTH);

    while(envp[i]){
        printf("evnp[%d] - %s\n",i,envp[i]);
        i++;
    }

    while(1){
        memset(str, 0, COMMAND_STRING_LENGTH);
        printf("[%s]$",ps1Prompt);
        scanf("%s",str);
        char *temp = str;
        while(*str){
        	if(*str == 10)
        		*str = '\0';
        	str++;
        }
        str = temp;
       	printf("str is: %s end\n",str);

        if(strcmp(str,"exit")==0){
            exit(0);
        }else if(strncmp(str,"cd",2)==0){
            path=malloc(strlen(str)-2);
            strcpy(path,str+3);
            printf("%s\n",path);
            returnres = chdir(path);
            if(returnres==-1)
                printf("Failed\n");
            free(path);
            path=NULL;
        }else if(strncmp(str,"export PS1=",11)==0){
            strcpy(ps1Prompt,str+11);
        }else if(strncmp(str,"PS1=",4)==0){
            strcpy(ps1Prompt,str+4);
        }else if(strncmp(str,"export PATH=",12)==0){
            pathvar = malloc(strlen(str)-6);
            strcpy(pathvar,str+7);
            setPath(pathvar,envp);
            free(pathvar);
            pathvar=NULL;
        }else if(strncmp(str,"PATH=",5)==0){
            pathvar = malloc(strlen(str)+1);
            strcpy(pathvar,str);
            setPath(pathvar,envp);
            free(pathvar);
            pathvar=NULL;
        }else{
        	if(!(strcmp(str,"")==0)){
        		commands = malloc(sizeof(char *)*NO_OF_COMMANDS);
                for(i=0;i<NO_OF_COMMANDS;i++)
                    commands[i] = NULL;
                noOfPipes = getCommands(str,commands);
				
				if(noOfPipes == 0){
					returnres = execute(commands,envp);
				}else{
					allCommands = malloc(sizeof(char **)*NO_OF_COMMANDS);
                	for (i=0; i<NO_OF_COMMANDS; i++) {
                   	 	allCommands[i] = NULL;
                	}

                	for (i = 0; i <= noOfPipes; i++){
                    	allCommands[i] = malloc(sizeof(char *)*NO_OF_ARGUMENTS);
                    	for (k = 0; k < NO_OF_ARGUMENTS; k++)
                        	allCommands[i][k] = NULL;
                    	getArguments(commands[i],allCommands[i]);
                    }
                    printf("Calling piping:%d\n",noOfPipes);
                	returnres = piping(noOfPipes,allCommands,envp);
                	printf("Piping returned:%d\n",returnres);

                	for (i = 0; allCommands[i]!=NULL; i++) {
                    	for (j=0; allCommands[i][j]!= NULL; j++){
                        	free(allCommands[i][j]);
                    	}
                    	free(allCommands[i]);
               		 }
                	free(allCommands); 
					
				} // pipes else

				if(returnres==-1){
                    printf("Failed\n");
                }        
                for(i=0;commands[i];i++){
                    free(commands[i]);
                }
                free(commands);
			} //if not new line end
        } //else end
    } // while end
    free(str);
    free(ps1Prompt);
} // main end
