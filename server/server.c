#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
 
#define BUF_SIZE 100

 
#pragma comment(lib, "ws2_32.lib")
 
void ErrorHandling(char* message);
char message[BUF_SIZE];
 
int main(int argc, char *argv[])
{
    WSADATA wsaData;
    SOCKET servSock;
    
    int strLen;
    int clntAdrSz;
 
    SOCKADDR_IN servAdr, clntAdr;
 
 	if(argc!=2)
    {
        printf("usage : %s <port>\n", argv[0]);
        exit(1);
    }
    
 	int PORT=atoi(argv[1]);
 	printf("PORT = %d\n",PORT);
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        ErrorHandling("WSAStartup() error");
 
    servSock = socket(PF_INET, SOCK_DGRAM, 0);
    if (servSock == INVALID_SOCKET)
        ErrorHandling("UDP socket creation error");
 
    memset(&servAdr, 0, sizeof(servAdr));
    servAdr.sin_family = AF_INET;
    servAdr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAdr.sin_port = htons(PORT);
 
    if (bind(servSock, (SOCKADDR*)&servAdr, sizeof(servAdr)) == SOCKET_ERROR)
        ErrorHandling("bind() error");
 	
 	
    while (1)
    {
        clntAdrSz = sizeof(clntAdr);
        strLen = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);
        if(strlen(message)!=0){
	        printf("Send Msg : %s   cliSize = %d\n",message,sizeof(clntAdr));
	        //for(int i=0;i<10000000;i++){}
	        //sendto(servSock, message, strlen(message)+1, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));
	        sendto(servSock, "hellow", strlen("hellow")+1, 0, (SOCKADDR*)&clntAdr, sizeof(clntAdr));
			if(strcmp(message,"exit")==0){
	        	printf("END!\n");
	        	break;
			}
			
		}
        
    }
 
    closesocket(servSock);
    WSACleanup();
    return 0;
 
}
 
void ErrorHandling(char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
    exit(1);
}
