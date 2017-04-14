#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include "socketLayer.h"

using namespace std;

#define BUFFER_SIZE 2048

long long sendFileToSocket(ifstream& is, SocketLayer &SL, SOCKET socket){
	is.seekg(0, ios::beg);
	char buffer[BUFFER_SIZE] = {};
	int sendCount = 0;
	long long calculatedFileSize = 0LL;
	do {
		memset(buffer, 0, sizeof(buffer));
		is.read((char *)&buffer, BUFFER_SIZE);
		if (is.eof()){
			is.clear();
			is.seekg(0, ios::end);
			long long realRestSize = is.tellg() - (long long)(sendCount*BUFFER_SIZE);
			cout << realRestSize << '\n';

			is.clear();
			is.seekg(-realRestSize, ios::end);
			memset(buffer, 0, sizeof(buffer));
			is.read((char *)&buffer, realRestSize);
			SL.Send(socket, buffer, realRestSize);
			calculatedFileSize += realRestSize;
			break;
		}
		else {
			Sleep(10);
			int byte = SL.Send(socket, buffer, BUFFER_SIZE);
			fprintf(stderr, "%2d] blockSize: %d / %d\n", ++sendCount, byte, BUFFER_SIZE);
			calculatedFileSize += BUFFER_SIZE;
		}
	} while (is.tellg() >= 0);
	is.close();
	return calculatedFileSize;
}

int main(int argc, char **argv){
	int port;
	char address[55];
	printf("IP: "); cin >> address;
	printf("PORT: "); cin >> port;
	printf("ip address : [%s:%d]\n", address, port);

	SocketLayer SL;
	SOCKET client;
	if( SL.Create(&client) == INVALID_SOCKET ){
		perror("FAIL TO CREATE SOCKET");
		return 1;
	}

	if (SL.Connect(client, SL.MakeAddress(address, port)) == SOCKET_ERROR){
		perror("FAIL TO CONNECT SOCKET");
		return 1;
	}

	SL.Send(client, "User Connected!", 15);

	char filename[BUFFER_SIZE] = {};
	printf("filename> ");
	fflush(stdin);
	while (fgets(filename, BUFFER_SIZE, stdin)){
		if (strncmp(filename, "quit\n", 5) == 0) break;

		filename[strlen(filename) - 1] = 0;
		printf("Send File.. \"%s\"\n", filename);

		ifstream inFile(filename, ios::in | ios::binary);
		if (!inFile.is_open()){
			fprintf(stderr, "File is not exists: \"%s\"\n", filename);
			continue;
		}

		char tag[BUFFER_SIZE + 30] = {};
		sprintf(tag, "<send-file>:%s", filename);
		SL.Send(client, tag, strlen(tag));
		sendFileToSocket(inFile, SL, client);
		SL.Send(client, "</send-file>", 12);

		char str[BUFFER_SIZE] = {};
		if( SL.Receive(client, str, BUFFER_SIZE) > 0 ){
			printf("Received Data from Server: %s\n", str);
		}
		printf("filename> ");
	}

	SL.Close(client);

	return 0;
}