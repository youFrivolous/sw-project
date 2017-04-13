#include <cstdio>
#include <iostream>
#include "socketLayer.h"

using namespace std;
 
#define MAX_PACKETLEN 512
 
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

		string buf;
        // try to receive some data, this is a blocking call
        if( SL.Receive(server, buf, 4096, clientSocketInfo) == SOCKET_ERROR ){
            printf("recvfrom() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        // print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(clientSocketInfo.sin_addr), ntohs(clientSocketInfo.sin_port));
        printf("Data: %s\n" , buf.data());

        // now reply the client with the same data
        if( SL.Send(server, buf, clientSocketInfo) == SOCKET_ERROR ){
            printf("sendto() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
	}
	SL.Close(server);

    return 0; 
 
} 