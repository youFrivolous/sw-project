#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <fstream>
using namespace std;

#pragma comment(lib, "ws2_32.lib") // Winsock Library

typedef struct sockaddr_in sockaddr_in;

#define STRING_LENGTH 512
#define BUFFER_SIZE 1<<16-1

void ErrorHandling(char* message) {
	fprintf(stderr, "%s : Error Code : \n", message, WSAGetLastError());
	WSACleanup();
	exit(EXIT_FAILURE);
}

int ServerSendToClient(bool isTCP, SOCKET& sock, char *buffer, int bufferSize, SOCKET& clientSock, sockaddr_in& echoAddr, int addrSize) {
	bool success = true;
	int msgSize = 0;
	if (isTCP) {
		msgSize = send(clientSock, buffer, bufferSize, 0);
	}
	else {
		msgSize = sendto(sock, buffer, bufferSize, 0, (sockaddr *)&echoAddr, addrSize);
	}

	success &= msgSize >= 0;
	if (success == false)
		ErrorHandling("ServerSendToClient sent a different number of bytes than expected");
	return msgSize;
}

int ServerReceiveFromClient(bool isTCP, SOCKET& sock, char *buffer, int bufferSize, SOCKET& clientSock, sockaddr_in& echoAddr, int *addrSize) {
	bool success = true;
	int msgSize = 0;
	if (isTCP) {
		msgSize = recv(clientSock, buffer, bufferSize, 0);
	}
	else {
		msgSize = recvfrom(sock, buffer, bufferSize, 0, (sockaddr *)&echoAddr, addrSize);
	}

	success &= msgSize >= 0;
	if (success == false)
		ErrorHandling("ServerReceiveFromClient sent a different number of bytes than expected");
	return msgSize;
}

int ClientSendToServer(bool isTCP, SOCKET& sock, char *buffer, int bufferSize, sockaddr_in& echoAddr, int addrSize) {
	bool success = true;
	int msgSize = 0;
	if (isTCP) {
		msgSize = send(sock, buffer, bufferSize, 0);
	}
	else {
		msgSize = sendto(sock, buffer, bufferSize, 0, (sockaddr *)&echoAddr, addrSize);
	}

	success &= msgSize >= 0;
	if( success == false )
		ErrorHandling("ClientSendToServer sent a different number of bytes than expected");
	return msgSize;
}

int ClientReceiveFromServer(bool isTCP, SOCKET& sock, char *buffer, int bufferSize, sockaddr_in& echoAddr, int *addrSize) {
	bool success = true;
	int msgSize = 0;
	if (isTCP) {
		msgSize = recv(sock, buffer, bufferSize, 0);
	}
	else {
		msgSize = recvfrom(sock, buffer, bufferSize, 0, (sockaddr *)&echoAddr, addrSize);
	}

	success &= msgSize >= 0;
	if (success == false)
		ErrorHandling("ClientReceiveFromServer receive a different number of bytes than expected");
	return msgSize;
}

int MakeMessage(char *buffer, char *message, int bufferSize = -1) {
	ZeroMemory(buffer, BUFFER_SIZE);
	int messageLength = bufferSize < 0 ? strlen(message) : bufferSize;
	strncpy(buffer, message, messageLength);
	return messageLength;
}

bool isBeginToSendFile(char* buffer, char* filename) {
	char token[] = "<send-file>";
	int tokenLength = strlen(token);
	if (strncmp(buffer, token, tokenLength) != 0) return false;

	int filenameLength = strlen(buffer + tokenLength);
	ZeroMemory(filename, filenameLength);
	strncpy(filename, buffer + tokenLength, filenameLength);
	filename[filenameLength] = 0;
	return true;
}
bool isEndToSendFile(char* buffer) {
	return strncmp(buffer, "</send-file>", strlen("</send-file>")) == 0;
}
long long getFileSizeFromBuffer(char* buffer) {
	char token[] = "<file-size>";
	int tokenLength = strlen(token);
	if (strncmp(buffer, token, tokenLength) != 0) return -1;

	long long fileSize = 0LL;
	int filesizeLength = strlen(buffer + tokenLength);
	for (int i = 0; i < filesizeLength; ++i) {
		fileSize *= 10LL;
		fileSize += (buffer[tokenLength + i] - '0');
	}
	return fileSize;
}

void checkSpeedAndPercentage(char *filename, long long *elapseFileSize, long long curFileSize, time_t *pastSecond, long long totalFileSize) {
	time_t now = clock(), elapseMS = now - *pastSecond;
	if (elapseMS >= 500) {
		double kbps = (*elapseFileSize / 1024LL) / (elapseMS + (double)1e-9);
		double percentage = curFileSize * 100LL / (totalFileSize + (double)1e-9);
		printf("%s]... kbps: %.3lf MB/s (%.2lf%%)\n", filename, kbps, percentage);
		*pastSecond = now;
		*elapseFileSize = 0LL;
	}
}

// return: fileSize
long long SendFileToServer(bool isTCP, char *filename, SOCKET& sock, sockaddr_in& echoAddr, int addrSize) {
	ifstream file(filename, ios::in | ios::binary);
	if (!file.is_open()) {
		fprintf(stderr, "File is not exists: \"%s\"\n", filename);
		return -1;
	}

	char buffer[BUFFER_SIZE] = {};
	sprintf(buffer, "<send-file>%s", filename);
	printf("SendFileToServer: %s\n", filename);
	ClientSendToServer(isTCP, sock, buffer, strlen(buffer), echoAddr, addrSize);

	long long totalFileSize = file.tellg();
	file.seekg(0, ios::end);
	totalFileSize = file.tellg() - totalFileSize;
	sprintf(buffer, "<file-size>%lld", totalFileSize);
	printf("Estimated File Size: %lld bytes\n", totalFileSize);
	ClientSendToServer(isTCP, sock, buffer, strlen(buffer), echoAddr, addrSize);

	file.seekg(0, ios::beg);
	int sendCount = 0;
	long long calculatedFileSize = 0LL;
	long long elapseFileSize = 0LL;
	time_t lastTickTime = clock();
	do {
		memset(buffer, 0, sizeof(buffer));
		file.read((char *)&buffer, BUFFER_SIZE);
		int recvSize = 0;
		if (file.eof()) {
			file.clear();
			file.seekg(0, ios::end);
			long long realRestSize = file.tellg() - (long long)(sendCount * BUFFER_SIZE);

			file.clear();
			file.seekg(-realRestSize, ios::end);
			memset(buffer, 0, sizeof(buffer));
			file.read((char *)&buffer, realRestSize);
			recvSize = ClientSendToServer(isTCP, sock, buffer, realRestSize, echoAddr, addrSize);
			break;
		}
		else {
			recvSize = ClientSendToServer(isTCP, sock, buffer, BUFFER_SIZE, echoAddr, addrSize);
			++sendCount;
		}
		calculatedFileSize += recvSize;
		elapseFileSize += recvSize;
		checkSpeedAndPercentage(filename, &elapseFileSize, calculatedFileSize, &lastTickTime, totalFileSize);
		Sleep(5);

	} while (file.tellg() >= 0);
	file.close();

	int msgLen = MakeMessage(buffer, "</send-file>");
	ClientSendToServer(isTCP, sock, buffer, msgLen, echoAddr, addrSize);

	return calculatedFileSize;
}

long long SaveFileToServer(bool isTCP, char *buffer, SOCKET& sock, SOCKET& clientSock, sockaddr_in& echoAddr, int* addrSize) {
	char filename[STRING_LENGTH] = {};
	if (isBeginToSendFile(buffer, filename)) {
		// <send-file> 이후 <file-size>
		ServerReceiveFromClient(isTCP, sock, buffer, BUFFER_SIZE, clientSock, echoAddr, addrSize);
		long long realFilesize = getFileSizeFromBuffer(buffer);

		printf("Processing File with \"%s\"\n", filename);
		ofstream file(filename, ios::out | ios::binary);
		int recvBlockSize = 0;
		time_t lastTickTime = clock();
		long long lastTickFileSize = 0LL;
		long long calculatedFileSize = 0LL;
		do {
			ZeroMemory(buffer, BUFFER_SIZE);
			/* Block until receive message from a client */
			recvBlockSize = ServerReceiveFromClient(isTCP, sock, buffer, BUFFER_SIZE, clientSock, echoAddr, addrSize);
			// if(recvBlockSize < BUFFER_SIZE) printf("recvBlockSize = %d\n", recvBlockSize);
			if (recvBlockSize <= 0) break;
			if (isEndToSendFile(buffer)) break;

			checkSpeedAndPercentage(filename, &lastTickFileSize, calculatedFileSize, &lastTickTime, realFilesize);
			lastTickFileSize += recvBlockSize;
			calculatedFileSize += recvBlockSize;
			file.write(buffer, recvBlockSize);
		} while (recvBlockSize > 0);
		file.close();

		int len = MakeMessage(buffer, "Successed to File Transfer\n");
		ServerSendToClient(isTCP, sock, buffer, len, clientSock, echoAddr, *addrSize);
		puts(buffer);
		return calculatedFileSize;
	}

	// this is not file sending packet
	return -1;
}