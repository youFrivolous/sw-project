#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <winsock2.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <io.h>
#define BUF_SIZE 100
using namespace std;

char dir[300], file[300];

void ErrorHandling(char* message);
int FindFile(char *dir,char *file);
int main()
{
	
    
    printf("DIR :"); 
	fflush(stdout);
	gets(dir);
    printf("FILE :");
	fflush(stdout);
	gets(file);
    //gets(file);
    _finddata_t fd;
    long handle = _findfirst(dir,&fd);
    if(handle==-1){
    	ErrorHandling((char*)"There were no files.");
	}
    
    
    if(FindFile(dir,file)){
    	printf("found %s\n",file);
	}
	else{
		printf("not found %s\n",file);
	}
    
	return 0;
}

void ErrorHandling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

int FindFile(char *dir,char *file){
	DIR *dp;
    struct dirent *dirp;
    
    
    
    dp=opendir(dir);
    while(dirp = readdir(dp)){
    	if(strcmp(dirp->d_name, file)==0){
    		closedir(dp);
    		return 1;
		}
	}
	closedir(dp);
	return 0;
}
