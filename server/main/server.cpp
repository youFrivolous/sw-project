#include <cstdio>
#include <iostream>
#include <fstream>
#include <ctime>
#include "socketLayer.h"

using namespace std;

bool askInformation(int& port) {
	// (warning) PORT_NUMBER is on socketLayer with global define
	port = PORT_NUMBER;
	return false;
}

int main() {
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
	while (true) {
		// need to accept if use tcp
		printf("Waiting for data...");

		fflush(stdout);
		ZeroMemory(echoBuffer, BUFFER_SIZE);
		/* Block until receive message from a client */
		if ((recvMsgSize = ServerReceiveFromClient(usingTCP, sock, echoBuffer, BUFFER_SIZE, clientSock, echoClntAddr, &cliLen)) == SOCKET_ERROR)
			ErrorHandling("recvfrom() failed");

		if (createServerDirectory(echoBuffer) == BEGIN_DIRECTORY) continue;

		if (isRequestForSwitchProtocol(echoBuffer, recvMsgSize)) {
			ServerResponeSwitchProtocol(&usingTCP, sock, echoServAddr, PORT);
			if (usingTCP) {
				ZeroMemory(&echoClntAddr, sizeof(echoClntAddr));
				if ((clientSock = accept(sock, (sockaddr *)&echoClntAddr, &cliLen)) == INVALID_SOCKET)
					ErrorHandling("Accept failed");
			}
		}

		printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

		recvMsgSize = SaveFileToServer(usingTCP, echoBuffer, sock, clientSock, echoClntAddr, &cliLen);
	}
	closesocket(clientSock);

	WSACleanup();  /* Cleanup Winsock */
	closesocket(sock);
	return 0;
}