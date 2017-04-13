#include <cstdio>
#include <string>
#include <iostream>
#include "socketLayer.h"

using namespace std;

#define MAXLEN 1024

int main(int argc, char **argv){
	int port;
	char address[MAXLEN];
	printf("IP: "); cin >> address;
	printf("PORT: "); cin >> port;
	printf("ip address : [%s:%d]\n", address, port);

	SocketLayer SL;
	SOCKET client;
	if( SL.Create(&client) == INVALID_SOCKET ){
		perror("FAIL TO CREATE SOCKET");
		return 1;
	}

	if( SL.Connect(client, SL.MakeAddress(address, port)) == SOCKET_ERROR ){
		perror("FAIL TO CONNECT SOCKET");
		return 1;
	}

	SL.Send(client, "User Connected!");

	char buffer[MAXLEN]={};
	printf("> ");
	while(fgets(buffer, MAXLEN-1, stdin)){
		if(strncmp(buffer, "quit\n", 5) == 0) break;

		SL.Send(client, buffer);

		string str;
		if( SL.Receive(client, str) > 0 ){
			printf("Received Data from Server: %s\n", str.data());
		}
		
		printf("> ");
	}

	SL.Close(client);

	return 0;
}