#include <cstdio>
#include <string>
#include <iostream>
#include "socketLayer.h"

using namespace std;

#define MAXLEN 1024

int main(int argc, char **argv){
	int port;
	char address[MAXLEN];
	string name;
	printf("IP: "); cin >> address;
	printf("PORT: "); cin >> port;
	printf("File Name: ");cin>>name;
	printf("ip address : [%s:%d] file name : %s\n", address, port,name.c_str());
	

	SocketLayer SL;
	SOCKET client;
	if( SL.Create(&client) == INVALID_SOCKET ){
		perror("FAIL TO CREATE SOCKET");
		return 1;
	}

	sockaddr_in addressInfo = SL.MakeAddress(address, port);
	if( SL.Connect(client, addressInfo) == SOCKET_ERROR ){
		perror("FAIL TO CONNECT SOCKET");
		return 1;
	}


	FILE* file = fopen(name.c_str(),"rb");
	if(file==NULL){
		perror("File is not exist\n");
		return 1;
	}
	
	SL.Send(client, name);
	
	char buffer[MAXLEN]={};
	while(fgets(buffer, MAXLEN, file) >0){
		SL.Send(client, buffer);
	}
	SL.Send(client,"~EXIT~");
	
	
	fclose(file);

	SL.Close(client);

	return 0;
}
