#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
 #define BUF_SIZE 100
void ErrorHandling(char* message);


int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET hSocket;
    SOCKADDR_IN servAddr;
 
    char message[100];
    int strLen;
    if(argc != 5)
    {
        printf("Usage : %s\n <IP> <PORT> <Path> <Path>", argv[0]);
    }
    
    
 	FILE *Read=fopen(argv[3],"r");
	FILE *Write=fopen(argv[4],"w");
 	char text_buf[BUF_SIZE+1]={};
 	
 	if(Read==NULL){
 		ErrorHandling("Read ERROR!");
 	}
 	
 	//Write=fopen(argv[4],"w");
 	
 	
 	printf("Usage : %s \n<IP>=%s \n<PORT>=%s \n<Path>=%s \n<Path>=%s", argv[0],argv[1],argv[2],argv[3],argv[4]);
 	
 	while(fgets(text_buf,BUF_SIZE,Read)){
 	 	fprintf(Write,"%s",text_buf);
	}
		
	fclose(Write);
	fclose(Read);
	
    closesocket(hSocket);
 
    // lib delete
    WSACleanup();
 
    return 0;
}
 
void ErrorHandling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
