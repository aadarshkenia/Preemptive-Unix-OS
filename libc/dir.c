#include <stdlib.h>
#include <string.h>


// #define MAX_BUFFER_SIZE 1024
struct dirent dirEntry;

void *opendir(const char *name){
	struct directory *dirPtr=NULL;
	int fileDesc = open(name,O_DIRECTORY);
	if(fileDesc == -1)
		return NULL;
	else{
		dirPtr = (struct directory *)malloc(sizeof(struct directory));
		if(dirPtr!=NULL)
			dirPtr->fd = fileDesc;
	}
	return (void *)dirPtr;
}

struct dirent *readdir(void *dir){
	struct directory *dirPtr = (struct directory *)dir;
	if(dirPtr == NULL)
		return NULL;
	struct dirent buffer;
	int bytesRead;

	bytesRead = getdents((unsigned int)dirPtr->fd,(struct dirent *)&buffer,sizeof(struct dirent));
	if(bytesRead == 0)
		return NULL;
	else{
		dirEntry.d_ino = buffer.d_ino;
		dirEntry.d_off = buffer.d_off;
		dirEntry.d_reclen = buffer.d_reclen;
		strcpy(dirEntry.d_name,buffer.d_name);
		return &dirEntry;
	}
}

int closedir(void *dir){
	struct directory *dirPtr = (struct directory *)dir;  
	if(dirPtr != NULL){
		close(dirPtr->fd);
		//free(dirPtr);
		return 0;
	}
	return -1;
}