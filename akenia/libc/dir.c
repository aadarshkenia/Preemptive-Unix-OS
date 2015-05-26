#include <stdlib.h>
#include <string.h>

struct dirent dirEntry;
struct  direct
{
    long d_ino;
    off_t d_off;
    unsigned short d_reclen;
    char d_name [NAME_MAX+1];
    uint16_t d_namelen;
};

void *opendir(const char *name){
	struct directory *dirPtr;
	int fileDesc = open(name,O_RDONLY);
	if(fileDesc == -1)
		return NULL;
	else
		dirPtr->fd = fileDesc;
	return dirPtr;
}
int closedir(void *dir){
	struct directory *dirPtr = (struct directory *)dir;  
	if(dirPtr != NULL){
		close(dirPtr->fd);
		free(dirPtr);
		return 0;
	}
	return -1;}

struct dirent *readdir(void *dir){
	struct direct dirBuffer;
	struct directory *dirPtr = (struct directory *)dir;

	while(read(dirPtr->fd, (char *)&dirBuffer, sizeof(dirBuffer)) == sizeof(dirBuffer)){
           if (dirBuffer.d_ino == 0) 
               continue;
           dirEntry.d_ino = dirBuffer.d_ino;
           strncpy(dirEntry.d_name, dirBuffer.d_name, dirBuffer.d_namelen);
           dirEntry.d_name[dirBuffer.d_namelen] = '\0';  
           dirEntry.d_reclen = dirBuffer.d_reclen;
           dirEntry.d_off = dirBuffer.d_off;
           return &dirEntry;
   	}
    return NULL;
}

