#include <cstdio>
#include <iostream>
#include <fstream>
#include "socketLayer.h"

using namespace std;
 
#define BUFFER_SIZE 2048
 
int main(){
	int port;
	SocketLayer *SL;
	printf("PORT: "); cin >> port;
	char suggestTCP[10];
	printf("Use TCP? (y/n): "); cin >> suggestTCP;
	bool usingTCP = suggestTCP[0] == 'Y' || suggestTCP[0] == 'y';
	if (usingTCP) {
		puts("TCP 프로토콜을 사용합니다.");
		SL = new SocketLayerTCP();
	}
	else {
		puts("UDP 프로토콜을 사용합니다.");
		SL = new SocketLayerUDP();
	}

	SOCKET server;
	if( SL->Create(&server) == INVALID_SOCKET ){
		puts("FAIL TO CREATE SOCKET");
	}
	cout<< "Success socket create at port " << port <<endl;
	
	// Prepare the sockaddr_in structure
	sockaddr_in socketInfo = SL->MakeAddress(port);
	int socketBlockSize = sizeof(socketInfo);

	if( SL->Bind(server, socketInfo) == SOCKET_ERROR ){
		perror("Bind Error");
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	if( SL->Listen(server, 1) == SOCKET_ERROR ){
		perror("Listen Error");
		exit(EXIT_FAILURE);
	}
	puts("Listen.....");

	bool connecting = false;
	SOCKET client;
	while( true ){
		sockaddr_in clientSocketInfo;
		if(connecting == false){
			ZeroMemory( &clientSocketInfo, sizeof(sockaddr_in) );
			client = SL->StartServer(server, clientSocketInfo);
			connecting = true;
		}

		char buf[BUFFER_SIZE] = {};
        // try to receive some data, this is a blocking call
		printf("입력 대기중....");
        if (SL->Receive(client, buf, BUFFER_SIZE) == SOCKET_ERROR){
			// retry to connection
			connecting = false;
			continue;
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
			while ((rsize = SL->Receive(client, buf, BUFFER_SIZE)) > 0){
				if (strncmp(buf, "</send-file>", 12) == 0){
					endFileReceive = true;
					break;
				}
				time_t now = clock(), elapseSecond = (now - lastTickTime) / CLOCKS_PER_SEC;
				if(elapseSecond >= 1){
					double kbps = (lastTickFileSize / 1024LL) / (elapseSecond + (double)1e-9);
					printf("%s]... kbps: %.3lf KB/s\n", filename, kbps);
					lastTickTime = now;
					lastTickFileSize = 0;
				}
				lastTickFileSize += rsize;
				outFile.write(buf, rsize);
			}
			outFile.close();
			printf("end!\n");
		}

		if(endFileReceive){
			// now reply the client with the result
			printf("convert hash to packet form\n");
			sprintf(buf, "<file-hash:%s", hashResult.c_str());
			printf("my hash is %s\n", hashResult.c_str());
			if (SL->Send(client, buf, BUFFER_SIZE) == SOCKET_ERROR) {
				printf("sendto() failed with error code : %d\n", WSAGetLastError());
				exit(EXIT_FAILURE);
			}
			puts("sended..");
		}
	}
	SL->Close(server);
	SL->Close(client);

	return 0; 
 
} 
