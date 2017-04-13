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
 	
 	FILE *Write=fopen("output.txt","w");
    while (1)
    {
        clntAdrSz = sizeof(clntAdr);
        strLen = recvfrom(servSock, message, BUF_SIZE, 0, (SOCKADDR*)&clntAdr, &clntAdrSz);
        if(strcmp(message,"EXITEXIT")==0){
	        printf("END!\n");
	        break;
		}
		else{
			fprintf(Write,"%s",message);
		}
    }
 	fclose(Write);
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
