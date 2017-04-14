#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <string>
#include <vector>
using namespace std;

#pragma comment(lib, "ws2_32.lib") // Winsock Library

typedef struct sockaddr_in sockaddr_in;

class SocketLayer {
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

	SOCKET Create(SOCKET *source = NULL){
		if( source == NULL ) source = new SOCKET;
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

	int Connect(SOCKET &socket, sockaddr_in &address){
		return connect(socket, (struct sockaddr*)&address, sizeof(address));
	}

	int Send(SOCKET &socket, string buffer){
		return send(socket, buffer.data(), buffer.length(), 0);
	}
	int Send(SOCKET &socket, string buffer, sockaddr_in &from){
		return sendto(socket, buffer.data(), buffer.length(), 0, (struct sockaddr *)&from, sizeof(sockaddr_in));
	}

	int Receive(SOCKET &socket, string &storage, const unsigned int buffer_length = 4096){
		vector<char> buffer(buffer_length);
		int receivedBytes = recv(socket, buffer.data(), buffer.size(), 0);
		if(receivedBytes == -1){
			// throw "Receive Error";
			return -1;
		}
		storage.assign( buffer.begin(), buffer.end() );
		return receivedBytes;
	}
	int Receive(SOCKET &socket, string &storage, const unsigned int buffer_length, sockaddr_in &from){
		vector<char> buffer(buffer_length);
		int blockSize = sizeof(sockaddr_in);
		int receivedBytes = recvfrom(socket, buffer.data(), buffer.size(), 0, (struct sockaddr *)&from, &blockSize);
		if(receivedBytes == -1){
			// throw "Receive Error";
			return -1;
		}
		storage.assign( buffer.begin(), buffer.end() );
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
