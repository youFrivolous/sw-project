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

char dir[300];

void ErrorHandling(char* message);
void FindFile(char *dir);
int main()
{
	
    printf("DIR :"); 
	fflush(stdout);
	gets(dir);
    //printf("FILE :");
	//fflush(stdout);
	//gets(file);
    //gets(file);
    _finddata_t fd;
    long handle = _findfirst(dir,&fd);
    if(handle==-1){
    	ErrorHandling((char*)"There were no files.");
	}
    
    FindFile(dir);
    
	return 0;
}

void ErrorHandling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void FindFile(char *dir){
	DIR *dp;
    struct dirent *dirp;
    
    int cnt=0;
    
    dp=opendir(dir);
    while(dirp = readdir(dp)){
    	if(strcmp(dirp->d_name,".")==0) continue;
    	if(strcmp(dirp->d_name,"..")==0) continue;
    	cnt++;
    	//printf("%s\n",dirp->d_name);
    	_finddata_t fd;
    	char dir_new[300]="";
    	strcpy(dir_new,dir);
    	strcat(dir_new,(char*)"\\");
    	strcat(dir_new,dirp->d_name);
	    long handle = _findfirst(dir_new,&fd);
	    if(handle!=-1){
	    	FindFile(dir_new);
		}
		
	}
	printf("%s : %s\n",dir,(cnt ? (char*)"dir":(char*)"file"));
	//printf("The Number of files : %d\n",cnt);
	
	closedir(dp);
}
