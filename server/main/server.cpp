#include <cstdio>
#include <iostream>
#include <fstream>
#include "socketLayer.h"

using namespace std;
 
#define BUFFER_SIZE 2048
 
int main(){
	int port;
	printf("PORT: "); cin >> port;

	SocketLayer SL;
	SOCKET server;
	if( SL.Create(&server) == INVALID_SOCKET ){
		puts("FAIL TO CREATE SOCKET");
	}
	cout<< "Success socket create at port " << port <<endl;
	
	// Prepare the sockaddr_in structure
	sockaddr_in socketInfo = SL.MakeAddress(port);
	int socketBlockSize = sizeof(socketInfo);

	if( SL.Bind(server, socketInfo) == SOCKET_ERROR ){
		perror("Bind Error");
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	while( true ){
		sockaddr_in clientSocketInfo;
        ZeroMemory( &clientSocketInfo, sizeof(sockaddr_in) ); 

		char buf[BUFFER_SIZE] = {};
        // try to receive some data, this is a blocking call
        if (SL.Receive(server, buf, BUFFER_SIZE, clientSocketInfo) == SOCKET_ERROR){
            printf("recvfrom() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
		}
		printf("[Receive fisrt: %s]\n", buf);

		// print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(clientSocketInfo.sin_addr), ntohs(clientSocketInfo.sin_port));
		if (strncmp(buf, "<send-file>", 11) == 0){
			char filename[BUFFER_SIZE] = {};
			strncpy(filename, buf + 12, BUFFER_SIZE - 12);
			printf("[%s]\n", filename);

			ofstream outFile(filename, ios::out | ios::binary);
			printf("File received: %s ...\n", filename);
			int rsize = 0, sendCount = 0;
			while ((rsize = SL.Receive(server, buf, BUFFER_SIZE, clientSocketInfo)) > 0){
				if (strncmp(buf, "</send-file>", strlen("</send-file>")) == 0) break;
				printf("%2d]... receive: %d\n", ++sendCount, rsize);
				outFile.write(buf, rsize);
			}
			outFile.close();
			printf("end!\n");
		}

        // now reply the client with the same data
		strcpy(buf, "I Got Your Request.");
        if( SL.Send(server, buf, BUFFER_SIZE, clientSocketInfo) == SOCKET_ERROR ){
            printf("sendto() failed with error code : %d\n" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
	}
	SL.Close(server);

	return 0; 
 
} 
