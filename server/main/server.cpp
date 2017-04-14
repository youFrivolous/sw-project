#include <cstdio>
#include <iostream>
#include "socketLayer.h"

using namespace std;
 
#define MAX_PACKETLEN 512
#define BUFSIZE 4096
 
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

	sockaddr_in clientSocketInfo;
	ZeroMemory( &clientSocketInfo, sizeof(sockaddr_in) ); 
	
	int rlen = 0;
	string buf, name;

	rlen = SL.Receive(server, name, 4096, clientSocketInfo);

	// print details of the client/peer and the data received
	printf("Received packet from %s:%d\n", inet_ntoa(clientSocketInfo.sin_addr), ntohs(clientSocketInfo.sin_port));
	printf("filename: %s\n", name.data());
	
	FILE* file=fopen(name.data(),"wb");
	
	while( (rlen = SL.Receive(server, buf, 4096, clientSocketInfo)) > 0 ){
		// print details of the client/peer and the data received
		printf("fileStream) from %s:%d\n", inet_ntoa(clientSocketInfo.sin_addr), ntohs(clientSocketInfo.sin_port));
		
		printf("%s\n", buf.data());
		fflush(stdout);
		if( strncmp(buf.data(), "~EXIT~", 6) == 0 ) break;
		
		fprintf(file,"%s", buf.data());
	}
	
	fclose(file);

	SL.Close(server);

	return 0; 
 
} 
