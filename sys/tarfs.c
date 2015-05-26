#include <sys/sbunix.h>
#include <sys/tarfs.h>
#include <sys/defs.h>
#include <string.h>
#include <sys/page.h>
#include <sys/structs.h>


#define MAX_LENGTH_FILENAME 128     // KAVANA
struct tar_file *tarfsroot=NULL;
// extern char * sys_getcwd(char *,size_t);
extern struct PCB *current_pcb;

uint64_t atoi(char *str)
{
    uint64_t res = 0, sign = 1;
    if (str[0] == '-') {
        sign = -1;
        str++;
    } else if (str[0] == '+') {
        sign = 1;
        str++;
    }
    while (*str) {
        if ( (uint64_t)(*str) >= 48 && (uint64_t)(*str) <= 57) {
            res = res*10+(*str)-'0';
            str++;
        } else 
            return 0;
    }
    return res*sign;
}

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

// int mystrcmp(const char *firstString,const char *secondString){
//     // printf("f:%s,s:%s\n",firstString,secondString);
//     while(*firstString && *secondString){
//         if(*firstString == *secondString){
//             firstString++;
//             secondString++;
//             continue;
//         }else
//             return *firstString - *secondString;
//     }
//     return *firstString - *secondString;
// }

uint64_t power(uint64_t x,uint64_t n)		// Change power function
{
    if(n == 0)
        return 1;
    else if (n%2 == 0)
        return power(x,n/2)*power(x,n/2);
    else
        return x*power(x,n/2)*power(x,n/2);
}

uint64_t convertOctalToDecimal(uint64_t octalNum){
    uint64_t decimalNum=0, i=0, r;
    uint64_t num = octalNum;
    while(num!=0)
    {
        r = num%10;
        decimalNum += r*power(8,i);
        i++;
        num	= num/10;
    }
    return decimalNum;
}

uint64_t alignFileUp(uint64_t size)
{
	uint64_t diff = FILE_CONTENT_SIZE - (size % FILE_CONTENT_SIZE);
	if(diff == FILE_CONTENT_SIZE)
		return size;
	else	
		return size + diff;
}

void * getFile(char * filename){

    struct tar_file * fileptr = lookUpForFile(filename); 
    if(!fileptr)
        return NULL;
    char *str = kmalloc(100);
    int i=0;
    int fsize = strlen(fileptr->filePath);
    for(i=1;i<fsize;i++)
    {
        *(str+i-1) = *(fileptr->filePath+i);

    }
    *(str+i-1)='\0';

	char *tarfObj = (char *)&_binary_tarfs_start;
 	
 	while(tarfObj < (char *)&_binary_tarfs_end){
 		struct posix_header_ustar *phuObj = (struct posix_header_ustar *) tarfObj;
        if(strcmp(phuObj->name,"") == 0){
            // printf("No such file\n");
            return NULL;
        }
 		uint64_t fileSize = convertOctalToDecimal(atoi(phuObj->size));
		//printf("name:%s\n",phuObj->name);
 	 	//printf("size:%d\n",fileSize);
	
		if(strcmp(str,phuObj->name) == 0){
			tarfObj = tarfObj + sizeof(struct posix_header_ustar);
			return (void *)tarfObj;
		}
	 	
 		if(fileSize == 0){
 			tarfObj = tarfObj + sizeof(struct posix_header_ustar);	
 		}
 		else if(fileSize > 0){
 			uint64_t alignedSize = alignFileUp(convertOctalToDecimal(atoi(phuObj->size)));
 	 		//printf("asize:%d\n",alignedSize);
  			tarfObj = tarfObj + alignedSize +  sizeof(struct posix_header_ustar);
  		}
 		//printf("tarfs:%x\n",tarfObj);
 	}
 	return NULL;	
}

// void printFiles(){

//     char *tarfObj = (char *)&_binary_tarfs_start;
    
//     while(tarfObj < (char *)&_binary_tarfs_end){

//         struct posix_header_ustar *phuObj = (struct posix_header_ustar *) tarfObj;
        // if(strcmp(phuObj->name,"") == 0){
        //     break;
        // }
//         uint64_t fileSize = convertOctalToDecimal(atoi(phuObj->size));
//         printf("name:%s\n",phuObj->name);        
        
//         if(fileSize == 0){
//             tarfObj = tarfObj + sizeof(struct posix_header_ustar);  
//         }
//         else if(fileSize > 0){
//             uint64_t alignedSize = alignFileUp(convertOctalToDecimal(atoi(phuObj->size)));
//             //printf("asize:%d\n",alignedSize);
//             tarfObj = tarfObj + alignedSize +  sizeof(struct posix_header_ustar);
//         }else{
//             printf("I AM HERE\n");
//         }
//         //printf("tarfs:%x\n",tarfObj);
//     }
// }

void parseFileName(const char *string,char **args,int *count){  
    int i=0,j=0,k=0;
    
    args[0] = (char *)kmalloc(sizeof(char)*MAX_LENGTH_FILENAME);
    for(i=0;string[i]!='\0';i++){
        if(string[i]!='/'){
            args[j][k] = string[i];
            k++;
        }else{
            args[j][k]='\0';
            j++;
            args[j] = (char *)kmalloc(sizeof(char)*MAX_LENGTH_FILENAME);
            k=0;
            (*count)++;
        }
    }
    if(string[i-1]!='/')
        (*count)++;
    args[j][k]='\0';
}
    
void setupHeirarchyOfTarfs(){
    
    tarfsroot = tar_file_alloc();
    tarfsroot->next = NULL;
    tarfsroot->child = NULL;
    tarfsroot->fileName = (char *)kmalloc(100);
    strcpy(tarfsroot->fileName,"/");
    tarfsroot->filePath = (char *)kmalloc(100);
    strcpy(tarfsroot->filePath,"/");
    tarfsroot->fileType = DIRTYPE;
    tarfsroot->fileSize = 0;
    tarfsroot->start = (uint64_t *)&_binary_tarfs_start;
    tarfsroot->end = (uint64_t *)&_binary_tarfs_end;
    tarfsroot->filePerm = convertOctalToDecimal(atoi("755"));
    tarfsroot->parent = NULL;
    
    char *tarfObj = (char *)&_binary_tarfs_start;

        while(tarfObj < (char *)&_binary_tarfs_end){
                struct posix_header_ustar *phuObj = (struct posix_header_ustar *) tarfObj;
                if(strcmp(phuObj->name,"") == 0){
                    break;
                }
                uint64_t fileSize = convertOctalToDecimal(atoi(phuObj->size));
                //printf("name:%s\n",phuObj->name);

                char **nodes=NULL;
                int count = 0;

				nodes = (char **)kmalloc(sizeof(char *) * MAX_LENGTH_FILENAME);
                for(int i=0;i < MAX_LENGTH_FILENAME;i++)
                    nodes[i] = NULL;

                parseFileName(phuObj->name,nodes,&count);

                // for (int i = 0; i<count; i++){
                    // printf("arg[%d]:%s\n",i,nodes[i]);
                // }

                int index = 0;
                // int nodeCount = count;

                if(count == 0)
                    printf("[tarfs setup heirarchy]Something is wrong\n");
                else{
                    struct tar_file *temp = tar_file_alloc();
                    temp->fileName = (char *)kmalloc(100);
                    strcpy(temp->fileName,nodes[count-1]);
                    temp->filePath = (char *)kmalloc(100);
                    strcpy(temp->filePath,"/");
                    mystrcat(temp->filePath,phuObj->name);
                    temp->fileSize = fileSize;
                    temp->fileType = atoi(phuObj->typeflag);
                    temp->next = NULL;
                    temp->child = NULL;
                    temp->start = (uint64_t *)(tarfObj + sizeof(struct posix_header_ustar));
                    temp->end = (uint64_t *)(tarfObj + sizeof(struct posix_header_ustar) + fileSize);
                    temp->filePerm = convertOctalToDecimal(atoi(phuObj->mode));
                    
 
                    struct tar_file *parentnode = tarfsroot;
                    count--;
                    while(count!=0){                // Go to the child level looking for right parent
                        parentnode = parentnode->child;
                        while(strcmp(parentnode->fileName,nodes[index])!=0)
                            parentnode = parentnode->next;
                        index++;
                        count--;
                    }
                    temp->parent = parentnode;
                    if(parentnode->child == NULL)   // Child level is reached , insert at appropriate sibling level
                        parentnode->child = temp;
                    else{
                        parentnode = parentnode->child;
                        while(parentnode->next != NULL)
                            parentnode = parentnode->next;
                        parentnode->next = temp;
                    }
                    // printf("filepath:%s\n",temp->filePath);
                }

                if(strcmp(phuObj->name,"lib/libc.a")==0){
                    break;
                }else{

                }
                
                if(fileSize == 0){
                        tarfObj = tarfObj + sizeof(struct posix_header_ustar);
                    }
                    else if(fileSize > 0){
                        uint64_t alignedSize = alignFileUp(convertOctalToDecimal(atoi(phuObj->size)));
                        //printf("asize:%d\n",alignedSize);
                        tarfObj = tarfObj + alignedSize +  sizeof(struct posix_header_ustar);
                }
                //printf("tarfs:%x\n",tarfObj);
        }
        
        //printf("node : %s\n", tarfsroot->child->fileName);
}


struct tar_file* lookUpForFile(const char *filePath){
    
    // printf("filePath:%s\n",filePath);
    // if(strcmp(filePath,"/")==0){
    if((strlen(filePath)==1) && (filePath[0]=='/')){
        return tarfsroot;
    }

    char **nodes=NULL;
    int count = 0;
    int index = 0;
    int isRelativePath=1;
    nodes = (char **)kmalloc(sizeof(char *) * MAX_LENGTH_FILENAME);
    for(int i=0;i < MAX_LENGTH_FILENAME;i++)
        nodes[i] = NULL;

    if(filePath[0] == '/'){
        isRelativePath=0;
        filePath++;
    }
    parseFileName(filePath,nodes,&count);

    if(isRelativePath){                         // Provided filePath is relative path
        char *curDir = (char *)kmalloc(sizeof(char)*MAX_LENGTH_FILENAME);
        strcpy(curDir,current_pcb->cwd);
        // char *buf = (char *)kmalloc(sizeof(char)*MAX_LENGTH_FILENAME);
        // char *curDir = sys_getcwd(buf,MAX_LENGTH_FILENAME);
        // printf("curdir [tarfs]:%s\n",curDir);
        // printf("value:%d\n",mystrcmp(curDir,"/"));

        struct tar_file *curNode = tarfsroot;
        // if(strcmp(curDir,"/")==0){              
        if((strlen(curDir)==1) && (curDir[0]=='/')){
            // Current directory is root
        }else{
            char **curDirPathNodes = NULL;
            int c=0,ind=0;
            curDirPathNodes = (char **)kmalloc(sizeof(char *) * MAX_LENGTH_FILENAME);
            for(int i=0;i < MAX_LENGTH_FILENAME;i++)
                curDirPathNodes[i] = NULL;
            curDir++;
            parseFileName(curDir,curDirPathNodes,&c);
            while(c!=0){                // Go to current directory
                curNode = curNode->child;
                while(curNode && (strcmp(curNode->fileName,curDirPathNodes[ind])!=0))
                    curNode = curNode->next;
                if(curNode == NULL){
                    printf("[tarfs lookup]Something is wrong. Invalid dir path returned from getcwd\n");
                    return NULL;
                }
                ind++;
                c--;
            }
        }
        while(count!=0){                // Go to the child level looking for right parent
            curNode = curNode->child;
            while(curNode && (strcmp(curNode->fileName,nodes[index])!=0))
                curNode = curNode->next;
            if(curNode == NULL){
                //printf("File doesn't exist\n");
                return NULL;
            }
            index++;
            count--;
        }
        return curNode;    
    }else{                                      // Provided filePath is absolute path
        if(count == 0){
            printf("[lookup tarfs]File name appears to be invalid\n");
            return NULL;    
        }
        else{
            struct tar_file *curNode = tarfsroot;
            while(count!=0){                // Go to the child level looking for right parent
                curNode = curNode->child;
                while(curNode && (strcmp(curNode->fileName,nodes[index])!=0))
                    curNode = curNode->next;
                if(curNode == NULL){
                    //printf("File doesn't exist\n");
                    return NULL;
                }
                index++;
                count--;
            }
            // printf("before return : %x\n", curNode->start);
            // printf("content : %s \n", ((char *)curNode->start));
            return curNode;
        }
    }
}