#include <cstdio>
#include <iostream>
#include <fstream>
#include <ctime>
#include "socketLayer.h"
#include "md5check.h"

using namespace std;

bool askInformation(int& port) {
	char tcpCheckStr[5] = {};
	printf("PORT: "); cin >> port;
	printf("using TCP? (y/n):"); cin >> tcpCheckStr;
	getchar(); fflush(stdin);
	return tcpCheckStr[0] == 'y' || tcpCheckStr[0] == 'Y';
}

void setUpSocket(bool isTCP, SOCKET& sock, sockaddr_in& address, int PORT) {
	if (isTCP) {
		if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
			ErrorHandling("socket() failed");
		puts("Socket created");

		address.sin_family = AF_INET;
		address.sin_port = htons(PORT);
		address.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

		if (bind(sock, (sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
			ErrorHandling("Bind Error\n");
		puts("Bind done");

		if (listen(sock, 5) == SOCKET_ERROR)
			ErrorHandling("listen Error\n");
		puts("Listen done");
	}
	else {
		/* Create socket for sending/receiving datagrams */
		if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
			ErrorHandling("socket() failed");
		puts("Socket created");

		/* Construct local address structure */
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = htonl(INADDR_ANY);
		address.sin_port = htons(PORT);

		/* Bind to the local address */
		if (bind(sock, (sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
			ErrorHandling("bind() failed");
		puts("Bind done");
	}
}

int main(){
	SOCKET sock;                       /* Socket */
	sockaddr_in echoServAddr;          /* Local address */
	sockaddr_in echoClntAddr;          /* Client address */
	char echoBuffer[BUFFER_SIZE];      /* Buffer for echo string */
	int cliLen = sizeof(echoClntAddr); /* Length of incoming message */
	int recvMsgSize;                   /* Size of received message */
	WSADATA wsaData;                   /* Structure for WinSock setup communication */
	int PORT;
	bool usingTCP = askInformation(PORT);

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) /* Load Winsock 2.0 DLL */
		ErrorHandling("WSAStartup() failed");

	setUpSocket(usingTCP, sock, echoServAddr, PORT);

	SOCKET clientSock;
	if (usingTCP) {
		ZeroMemory(&echoClntAddr, sizeof(echoClntAddr));
		if ((clientSock = accept(sock, (sockaddr *)&echoClntAddr, &cliLen)) == INVALID_SOCKET)
			ErrorHandling("Accept failed");
	}

	/* Run forever */
	while(true){
		// need to accept if use tcp

		printf("Waiting for data...");
		fflush(stdout);

		ZeroMemory(echoBuffer, BUFFER_SIZE);
		/* Block until receive message from a client */
		if ((recvMsgSize = ServerReceiveFromClient(usingTCP, sock, echoBuffer, BUFFER_SIZE, clientSock, echoClntAddr, &cliLen)) == SOCKET_ERROR)
			ErrorHandling("recvfrom() failed");

		printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

		recvMsgSize = SaveFileToServer(usingTCP, echoBuffer, sock, clientSock, echoClntAddr, &cliLen);
	}
	closesocket(clientSock);

	WSACleanup();  /* Cleanup Winsock */
	closesocket(sock);
	return 0;
} 
