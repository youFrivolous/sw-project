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
    if(argc != 4)
    {
        printf("Usage : %s\n <IP> <PORT> <Path>", argv[0]);
    }
 
    // socket library reset
    if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
        ErrorHandling("WSAStartip() error");
 	
    // socket make
    hSocket = socket(PF_INET, SOCK_DGRAM, 0);
    if(hSocket == INVALID_SOCKET)
        ErrorHandling("sock() error");
 
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = inet_addr(argv[1]);
    //servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servAddr.sin_port = htons(atoi(argv[2]));
 	
 	//strLen=strlen(argv[3]);
 	//strcpy(message,argv[3]);
    
 	//data send
 	printf("SEND!!!!%s\n",message);
 	
 	FILE *Read=fopen(argv[3],"r");
 	char text_buf[BUF_SIZE+1];
 	
 	if(Read==NULL){
 		ErrorHandling("Read ERROR!");
 	}
 	
 	
 	while(fgets(text_buf,BUF_SIZE,Read)){
	
	 	
	 	sendto(hSocket, text_buf, strlen(text_buf)+1, 0, (SOCKADDR*)&servAddr, sizeof(servAddr));
	 	
 	}
 	sendto(hSocket, "EXITEXIT", strlen("EXITEXIT")+1, 0, (SOCKADDR*)&servAddr, sizeof(servAddr));
 	
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
