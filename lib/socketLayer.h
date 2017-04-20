#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <WS2tcpip.h> // TCP/IP
#include <string>
#include <vector>
using namespace std;

#pragma comment(lib, "ws2_32.lib") // Winsock Library

typedef struct sockaddr_in sockaddr_in;

class SocketLayer {
public:
	static unsigned int const BUFFER_SIZE = 4096;
public:
	SocketLayer(){
		if(WSAStartup(MAKEWORD(2,2), &wsaData) != NO_ERROR) {
			fprintf(stderr, "Failed. Error Code : %d", WSAGetLastError());
			throw "WASStartup Error";
		}
	}
	~SocketLayer(){
		WSACleanup();
	}

	SOCKET Create(SOCKET *source = NULL){ }

	sockaddr_in MakeAddress(string destIP, int port){ }

	sockaddr_in MakeAddress(int port){ }

	int Connect(SOCKET &socket, sockaddr_in &address){
		return connect(socket, (struct sockaddr*)&address, sizeof(address));
	}

	int Send(SOCKET &socket, char * buffer, unsigned int buffer_length){
		return send(socket, buffer, buffer_length, 0);
	}
	int Send(SOCKET &socket, char * buffer, unsigned int buffer_length, sockaddr_in &from){
		return sendto(socket, buffer, buffer_length, 0, (struct sockaddr *)&from, sizeof(sockaddr_in));
	}

	int Receive(SOCKET &socket, char * buffer, const unsigned int buffer_length){
		int receivedBytes = recv(socket, buffer, buffer_length, 0);
		if(receivedBytes == -1){
			// throw "Receive Error";
			return -1;
    }
		return receivedBytes;
	}
	int Receive(SOCKET &socket, char * buffer, const unsigned int buffer_length, sockaddr_in &from){
		int blockSize = sizeof(sockaddr_in);
		int receivedBytes = recvfrom(socket, buffer, buffer_length, 0, (struct sockaddr *)&from, &blockSize);
		if(receivedBytes == -1){
			// throw "Receive Error";
			return -1;
		}
		return receivedBytes;
	}

	int Close(SOCKET &socket){
		return closesocket(socket);
	}

	int Bind(SOCKET &socket, sockaddr_in &socketInfo){
		return bind(socket,  (sockaddr*)&socketInfo, sizeof(sockaddr_in));
	}

	int Listen(SOCKET &socket, int backlog = 5){
		return listen(socket, 5);
	}

	int Accept(SOCKET &socket, sockaddr_in &socketInfo, int *socketLength){
		return accept(socket, (struct sockaddr*)&socketInfo, socketLength );
	}

private:
  WSADATA wsaData;

  SOCKADDR_IN serverAddress;
	SOCKADDR_IN clientAddress;
};

class SocketLayerUDP : public SocketLayer {
public:
	SOCKET Create(SOCKET *source = NULL){
		if(source == NULL) source = new SOCKET;
		*source = socket(AF_INET, SOCK_DGRAM, 0);
		return *source;
	}

	sockaddr_in MakeAddress(string destIP, int port){
		sockaddr_in addr;
		memset((void *)&addr, 0x00, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.S_un.S_addr = inet_addr(destIP.data());
		addr.sin_port = htons(port);
		return addr;
	}

	sockaddr_in MakeAddress(int port){
		sockaddr_in addr;
		memset((void *)&addr, 0x00, sizeof(addr));
		addr.sin_family = AF_INET; 
		addr.sin_addr.s_addr = INADDR_ANY; // 모든 사용 가능한 주소로부터 기다리겠다는 의미
		addr.sin_port = htons( port ); 
		return addr;
	}
};

class SocketLayerTCP : public SocketLayer {
public:
	SOCKET Create(SOCKET *source = NULL){
		if(source == NULL) source = new SOCKET;
		*source = socket(AF_INET, SOCK_STREAM, 0);
		return *source;
	}

	sockaddr_in MakeAddress(string destIP, int port){
		sockaddr_in addr;
		memset((void *)&addr, 0x00, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		inet_pton(AF_INET, destIP.data(), &addr.sin_addr.s_addr);
		return addr;
	}

	sockaddr_in MakeAddress(int port){
		sockaddr_in addr;
		memset((void *)&addr, 0x00, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);  // 현재 PC IP주소 사용
		addr.sin_port = htons(port);        // 포트번호
		return addr;
	}

	bool Listen(SOCKET& serverSocket, int backlog = 1){
		return listen(serverSocket, backlog) != SOCKET_ERROR;
	}

	SOCKET Accept(SOCKET& serverSocket, sockaddr_in &socketInfo){
		int addrLength = sizeof(socketInfo);
		return accept(serverSocket, (struct sockaddr*)&socketInfo, &addrLength);
	}

	SOCKET StartServer(SOCKET& serverSocket, sockaddr_in& clientSocketInfo){
		SOCKET cli = Accept(serverSocket, clientSocketInfo);
		if(cli == INVALID_SOCKET){
			perror("Accept Error");
			exit(EXIT_FAILURE);
		} else {
			printf("%s Connection Complete!\n", inet_ntoa(clientSocketInfo.sin_addr));
			printf("Start ...\n");
		}
		return cli;
	}
};