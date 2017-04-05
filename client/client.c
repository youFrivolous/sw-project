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
        printf("Usage : %s\n <IP> <PORT> <Msg>", argv[0]);
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
 	
 	strLen=strlen(argv[3]);
 	strcpy(message,argv[3]);
    
 	//data send
 	printf("SEND!!!!%s\n",message);
 	
 	while(1){
	
	 	
	 	sendto(hSocket, message, strlen(message)+1, 0, (SOCKADDR*)&servAddr, sizeof(servAddr));
	 	
	    // data recv
	    char MSG[BUF_SIZE]={};
	    int servAddrSz;
	    servAddrSz = sizeof(servAddr);
	    strLen = recvfrom(hSocket, MSG, BUF_SIZE, 0, (SOCKADDR*)&servAddr, &servAddrSz);
	    //printf("1:%s 2:%s\n",message,MSG);
	    //if(strLen == -1)ErrorHandling("read() error");
	    //if(strcmp(message,MSG)==0){
	    if(strcmp("hellow",MSG)==0){
			printf("Message from server : %s \n", MSG);
	    	break;
		}
 	
 	}
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
