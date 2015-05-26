#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char* argv[], char* envp[]){

    //int pid;
    char *s = (char *)malloc(20);

    while(1)
    {
        scanf("%s",s);
        
        pid=fork();
        if(!pid)
        {

        }
        else
        {
            pid=execve(s,argv,envp);
        }
    }


    return 0;
} // main end
