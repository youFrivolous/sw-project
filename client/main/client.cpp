#include <cstdio>
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#include "socketLayer.h"

using namespace std;

bool askInformation(char *ip, int& port) {
	// char tcpCheckStr[5] = {};
	printf("IP: "); cin >> ip;
	// printf("PORT: "); cin >> port;
	port = PORT_NUMBER;
	// printf("using TCP? (y/n):"); cin >> tcpCheckStr;
	getchar(); fflush(stdin);
	// return tcpCheckStr[0] == 'y' || tcpCheckStr[0] == 'Y';
	return false;
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

	setUpSocket(usingTCP, sock, echoServAddr, PORT, servIP);
	AddrIP(servIP);

	do {
		fflush(stdout);
		printf("File or Directory Name> ");
		fgets(echoString, STRING_LENGTH, stdin);

		echoString[strlen(echoString) - 1] = 0;

		/* Send the string, including the null terminator, to the server */
		long long fileSize = 0LL;
		if (dirExists(echoString))
			fileSize = SendDirectoryToServer(usingTCP, echoString, sock, echoServAddr, fromSize);
		else {
			fileSize = SendFileToServer(usingTCP, NULL, echoString, sock, echoServAddr, fromSize);
			continue;
		}
		if (fileSize <= 0) continue;

		/* Recv a response */
		ZeroMemory(echoBuffer, BUFFER_SIZE);
		if (ClientReceiveFromServer(usingTCP, sock, echoBuffer, BUFFER_SIZE, fromAddr, &fromSize) == SOCKET_ERROR)
			ErrorHandling("recvfrom() failed");

		printf("Result: %s\n", echoBuffer);    /* Print the echoed arg */

	} while (true);

	closesocket(sock);

	WSACleanup();  /* Cleanup Winsock */

	return 0;
}