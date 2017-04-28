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
	printf("using TCP? (y/n):"); cin >> tcpCheckStr;
	getchar(); fflush(stdin);
	return tcpCheckStr[0] == 'y' || tcpCheckStr[0] == 'Y';
}

void setUpSocket(bool isTCP, SOCKET& sock, sockaddr_in& address, char *servIP, int PORT) {
	if (isTCP) {
		if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET)
			ErrorHandling("socket() failed");

		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = inet_addr(servIP);
		address.sin_port = htons(PORT);

		if (connect(sock, (sockaddr *)&address, sizeof(address)) == SOCKET_ERROR)
			ErrorHandling("connect failed");
	}
	else {
		/* Create a best-effort datagram socket using UDP */
		if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) == INVALID_SOCKET)
			ErrorHandling("socket() failed");

		/* Construct the server address structure */
		memset(&address, 0, sizeof(address));    /* Zero out structure */
		address.sin_family = AF_INET;                 /* Internet address family */
		address.sin_addr.S_un.S_addr = inet_addr(servIP);  /* Server IP address */
		address.sin_port = htons(PORT);               /* Server port */
	}
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

	setUpSocket(usingTCP, sock, echoServAddr, servIP, PORT);

	do {
		fflush(stdout);
		printf("파일 또는 디렉토리명> ");
		fgets(echoString, STRING_LENGTH, stdin);
		echoString[strlen(echoString) - 1] = 0;

		/* Send the string, including the null terminator, to the server */
		long long fileSize = 0LL;
		if (dirExists(echoString))
			fileSize = SendDirectoryToServer(usingTCP, echoString, sock, echoServAddr, fromSize);
		else
			fileSize = SendFileToServer(usingTCP, echoString, sock, echoServAddr, fromSize);
		if (fileSize <= 0) continue;

		/* Recv a response */
		ZeroMemory(echoBuffer, BUFFER_SIZE);
		if (ClientReceiveFromServer(usingTCP, sock, echoBuffer, BUFFER_SIZE, fromAddr, &fromSize) == SOCKET_ERROR)
			ErrorHandling("recvfrom() failed");

		printf("Received: %s\n", echoBuffer);    /* Print the echoed arg */

	} while (true);

	closesocket(sock);

	WSACleanup();  /* Cleanup Winsock */

	return 0;
}