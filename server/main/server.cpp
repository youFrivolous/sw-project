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
	// printf("using TCP? (y/n):"); cin >> tcpCheckStr;
	getchar(); fflush(stdin);
	return tcpCheckStr[0] == 'y' || tcpCheckStr[0] == 'Y';
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

	/* Create socket for sending/receiving datagrams */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		ErrorHandling("socket() failed");
	puts("Socket created");

	/* Construct local address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));
	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	echoServAddr.sin_port = htons(PORT);

	/* Bind to the local address */
	if (bind(sock, (sockaddr *) &echoServAddr, sizeof(echoServAddr)) == SOCKET_ERROR)
		ErrorHandling("bind() failed");
	puts("Bind done");

	while(true){ /* Run forever */
		printf("Waiting for data...");
		fflush(stdout);

		ZeroMemory(echoBuffer, BUFFER_SIZE);
		/* Block until receive message from a client */
		if ((recvMsgSize = recvfrom(sock, echoBuffer, BUFFER_SIZE, 0, (sockaddr *)&echoClntAddr, &cliLen)) == SOCKET_ERROR)
			ErrorHandling("recvfrom() failed");

		printf("Handling client %s\n", inet_ntoa(echoClntAddr.sin_addr));

		recvMsgSize = SaveFileToServer(usingTCP, echoBuffer, sock, echoClntAddr, &cliLen);
	}

	WSACleanup();  /* Cleanup Winsock */
	closesocket(sock);
	return 0;
} 
