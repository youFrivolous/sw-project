#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include "md5check.h"
#include "socketLayer.h"

using namespace std;

bool askInformation(char *ip, int& port) {
	char tcpCheckStr[5] = {};
	printf("IP: "); cin >> ip;
	printf("PORT: "); cin >> port;
	// printf("using TCP? (y/n):"); cin >> tcpCheckStr;
	getchar(); fflush(stdin);
	return tcpCheckStr[0] == 'y' || tcpCheckStr[0] == 'Y';
}

int main() {
	SOCKET sock;                        /* Socket descriptor */
	sockaddr_in echoServAddr; /* Echo server address */
	sockaddr_in fromAddr;     /* Source address of echo */
	int echoServPort;     /* Echo server port */
	int fromSize = sizeof(fromAddr); /* In-out of address size for recvfrom() */
	char servIP[STRING_LENGTH];      /* IP address of server */
	char echoString[BUFFER_SIZE];    /* String to send to echo server */
	char echoBuffer[BUFFER_SIZE];    /* Buffer for echo string */
	int respStringLen;               /* Length of response string */
	WSADATA wsaData;                 /* Structure for WinSock setup communication */
	int PORT;
	bool usingTCP = askInformation(servIP, PORT);

	if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) { /* Load Winsock 2.0 DLL */
		ErrorHandling("WSAStartup() failed");
	}

	/* Create a best-effort datagram socket using UDP */
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
		ErrorHandling("socket() failed");

	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));    /* Zero out structure */
	echoServAddr.sin_family = AF_INET;                 /* Internet address family */
	echoServAddr.sin_addr.S_un.S_addr = inet_addr(servIP);  /* Server IP address */
	echoServAddr.sin_port = htons(PORT);               /* Server port */

	do {
		fflush(stdout);
		printf("ÆÄÀÏ¸í> ");
		fgets(echoString, STRING_LENGTH, stdin);
		echoString[strlen(echoString) - 1] = 0;

		/* Send the string, including the null terminator, to the server */
		long long fileSize = SendFileToServer(usingTCP, echoString, sock, echoServAddr, fromSize);
		if (fileSize <= 0) continue;
		// ClientSendToServer(usingTCP, sock, echoString, strlen(echoString), echoServAddr, fromSize);

		/* Recv a response */
		ZeroMemory(echoBuffer, BUFFER_SIZE);
		if (recvfrom(sock, echoBuffer, BUFFER_SIZE, 0, (sockaddr *)&fromAddr, &fromSize) == SOCKET_ERROR)
			ErrorHandling("recvfrom() failed");

		printf("Received: %s\n", echoBuffer);    /* Print the echoed arg */
	} while (true);
	closesocket(sock);
	WSACleanup();  /* Cleanup Winsock */

	return 0;
}