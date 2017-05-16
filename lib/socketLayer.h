#pragma once

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include <set>
#include <fstream>

#include <io.h>
#include <direct.h>
#include <Windows.h>
#define _WIN32_WINNT 0x0500
#include <Sddl.h>

#include "md5check.h"

using namespace std;

#pragma comment(lib, "ws2_32.lib") // Winsock Library

typedef struct sockaddr_in sockaddr_in;

#define STRING_LENGTH      512
#define BUFFER_SIZE        1<<16-1
#define UDP_FILESIZE_LIMIT 2*BUFFER_SIZE /* KB */

#define BEGIN_FILE      0x0000
#define END_FILE        0x0001
#define BEGIN_FILENAME  0x0010
#define END_FILENAME    0x0011	
#define BEGIN_FILESIZE  0x0100
#define END_FILESIZE    0x0101
#define BEGIN_DIRECTORY 0x0110
#define END_DIRECTORY   0x0111

#define PORT_NUMBER     8888

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
	if (strncmp(buffer, token, tokenLength) != 0) return -1LL;

	long long fileSize = 0LL;
	for (int i = tokenLength; buffer[i]; ++i) {
		if (buffer[i] < '0' || buffer[i] > '9') break;
		fileSize *= 10LL;
		fileSize += (int)(buffer[i] - '0');
	}
	return fileSize;
}

string extractHash(const char* buffer){
	char token[] = "</send-file>";
	int tokenLength = strlen(token);
	if(strncmp(buffer, token, tokenLength) != 0) return "(failed)";

	return buffer + tokenLength;
}

long long getResendFileSize(const char* buffer, int bufferSize) {
	char token[] = "<retry-from>";
	int tokenLength = strlen(token);
	long long result = 0LL;
	if (strncmp(buffer, token, tokenLength) == 0) {
		for (int i = tokenLength; buffer[i]; ++i) {
			if (buffer[i] < '0' || buffer[i] > '9') break;
			result = result * 10LL + (buffer[i] - '0');
		}
	}
	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 소켓 생성에 대한 함수들의 정의
// 프로토콜 변경에 대처하기 위해 공통된 인터페이스를 가진다.
///////////////////////////////////////////////////////////////////////////////////////////////////

// 소켓을 생성하는 함수 (서버-사이드)
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

// 소켓을 생성하는 함수 (클라이언트-사이드)
void setUpSocket(bool isTCP, SOCKET& sock, sockaddr_in& address, int PORT, char *servIP) {
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
		memset(&address, 0, sizeof(address));         /* Zero out structure */
		address.sin_family = AF_INET;                 /* Internet address family */
		address.sin_addr.S_un.S_addr = inet_addr(servIP);  /* Server IP address */
		address.sin_port = htons(PORT);               /* Server port */
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 프로토콜 변경에 대한 함수들
///////////////////////////////////////////////////////////////////////////////////////////////////

// 클라이언트의 프로토콜을 변경한다.
// 변경하기 전, UDP로 연결중일 서버에게도 변경을 요청한다.
// 서버가 변경된 이후에 다시 클라이언트가 변경된 프로토콜로 접속을 시도한다.
void ClientRequestSwitchProtocol(bool* isTCP, SOCKET& sock, sockaddr_in& address, int PORT, char *servIP) {
	printf("Request for switching Protocol: %s ... ", *isTCP ? "tcp -> udp" : "udp -> tcp");

	// 기존에 연결된 방식으로 변경을 요청한다.
	char buffer[BUFFER_SIZE] = "<request-switch-protocol>";
	ClientSendToServer(*isTCP, sock, buffer, sizeof(buffer), address, sizeof(address));

	// 변경을 요청한 후, 클라이언트가 먼저 변경한다.
	closesocket(sock);
	*isTCP = !(*isTCP);
	Sleep(1000); // 서버가 변경되기를 잠시 기다려보자.

				 // 변경된 프로토콜로 연결 시도
	// puts("Try to reconnect with switched protocol..");
	setUpSocket(*isTCP, sock, address, PORT, servIP);
	printf("Complete!(%s)\n", *isTCP ? "tcp" : "udp");
}

bool isRequestForSwitchProtocol(char *buffer, int bufferSize) {
	char token[] = "<request-switch-protocol>";
	int len = strlen(token);
	return (strncmp(buffer, token, len) == 0);
}

void ServerResponeSwitchProtocol(bool* isTCP, SOCKET& sock, sockaddr_in& address, int PORT) {
	printf("Request for switching Protocol: %s ... ", *isTCP ? "tcp -> udp" : "udp -> tcp");
	closesocket(sock);
	*isTCP = !(*isTCP);
	setUpSocket(*isTCP, sock, address, PORT);
	printf("Complete!(%s)\n", *isTCP ? "tcp" : "udp");
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// DIRECTORY
///////////////////////////////////////////////////////////////////////////////////////////////////

bool dirExists(const std::string& dirName_in){
	DWORD ftyp = GetFileAttributesA(dirName_in.c_str());
	if (ftyp == INVALID_FILE_ATTRIBUTES)
		return false;  //something is wrong with your path!

	if (ftyp & FILE_ATTRIBUTE_DIRECTORY)
		return true;   // this is a directory!
	return false;    // this is not a directory!
}


SECURITY_DESCRIPTOR *get_security_descriptor(char *filename) {
	/* First find out the amount of memory needed for the security descriptor */
	SECURITY_DESCRIPTOR *sd;
	unsigned long size = 0;

	GetFileSecurity(LPCWSTR(filename), DACL_SECURITY_INFORMATION, 0, 0, &size);

	/* Allocate memory for security descriptor */
	sd = (SECURITY_DESCRIPTOR *)HeapAlloc(GetProcessHeap(), 0, size);
	if (!sd) {
		fprintf(stderr, "Out of memory for security descriptor!\n");
		return 0;
	}

	/* Now perform the actual GetFileSecurity() call */
	if (!GetFileSecurity(LPCWSTR(filename), 0, sd, size, &size)) {
		fprintf(stderr, "Error getting file security DACL: %d\n", GetLastError());
		HeapFree(GetProcessHeap(), 0, sd);
		return 0;
	}

	return sd;
}

bool isEnterableDirectory(_finddata_t& fileinfo) {
	bool access = true;
	access &= (fileinfo.attrib & _A_SUBDIR) != 0;
	access &= (fileinfo.attrib & _A_SYSTEM) == 0;
	access &= (fileinfo.attrib & _A_HIDDEN) == 0;
	return access;
}

class FilePath {
public:
	FilePath() : isDir(false) {}
	FilePath(string dir, string name, bool isDirectory) : directory(dir), filename(name), isDir(isDirectory) {}
public:
	std::string directory;
	std::string filename;
	bool isDir;
};

bool getDirectoryList(std::string dir, vector<FilePath> *paths)
{
	char originalDirectory[_MAX_PATH];

	// Get the current directory so we can return to it
	_getcwd(originalDirectory, _MAX_PATH);

	_chdir(dir.c_str());  // Change to the working directory
	_finddata_t fileinfo;

	// This will grab the first file in the directory
	// "*" can be changed if you only want to look for specific files
	intptr_t handle = _findfirst("*.*", &fileinfo);

	if (handle == -1) { // No files or directories found
		perror("Error searching for file");
		exit(1);
	}

	do {
		if (strcmp(fileinfo.name, ".") == 0 || strcmp(fileinfo.name, "..") == 0)
			continue;

		bool isSubDir = isEnterableDirectory(fileinfo);
		FilePath fp(dir, fileinfo.name, isSubDir);
		paths->push_back(fp);

		if (isSubDir) { // Use bitmask to see if this is a directory
			string subDir(dir);
			subDir.append(fileinfo.name);
			getDirectoryList(subDir + "\\", paths);
		}
	} while (_findnext(handle, &fileinfo) == 0);

	_findclose(handle); // Close the stream
	_chdir(originalDirectory);
	return true;
}

string getRealPath(const char* filePath) {
	// 입력을 상대경로로 가정하고, 절대경로로 변경
	char originalDirectory[_MAX_PATH];
	_getcwd(originalDirectory, _MAX_PATH);
	string currentDir(originalDirectory);
	currentDir.append("\\");
	currentDir.append(filePath);
	currentDir.append("\\");
	return currentDir;
}

int createServerDirectory(char *buffer) {
	char token[] = "<create-dir>";
	int tokenLength = strlen(token);
	if (strncmp(buffer, token, tokenLength) != 0) return -1;
	printf("\nRequest Creating Directory: \"%s\"\n", buffer + tokenLength);

	// 버퍼에 파일이 여러개의 Send가 누적되어 있을 수 있다.
	// 자세한 이유는 http://egloos.zum.com/depiness/v/911099
	// 따라서 중간에 끼어있는 구분자를 통해 자른다.
	string bufferedStr(buffer + tokenLength);
	int lastTokenIndex = 0, bufferLength = bufferedStr.length();
	for(int i=0; i <= bufferLength; ++i){
		// <create-file> 의 앞머리가 나온것으로 추측한다.
		if (bufferedStr[i] == '<' || i == bufferLength) {
			string filepath(getRealPath(bufferedStr.substr(lastTokenIndex, i - lastTokenIndex).c_str()));
			_mkdir(filepath.c_str());
			if (i == bufferLength) break;

			if (strncmp(bufferedStr.c_str() + i, token, tokenLength) == 0) {
				i += tokenLength - 1;
			}
			lastTokenIndex = i + 1;
		}
	}

	return BEGIN_DIRECTORY; /* Catch Packet And Make Directory */
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// FILE TRANSFER
///////////////////////////////////////////////////////////////////////////////////////////////////

void checkSpeedAndPercentage(const char *filename, long long *elapseFileSize, long long curFileSize, time_t *pastSecond, long long totalFileSize) {
	time_t now = clock(), elapseMS = now - *pastSecond;
	if (elapseMS >= 500) {
		double kbps = (*elapseFileSize / 1024LL) / (elapseMS + (double)1e-9);
		printf("%s]... kbps: %.3lf MB/s ", filename, kbps);
		if (totalFileSize > 0LL) {
			// totalFileSize 가 -1 인 경우는, <file-size>를 알지 못했다는 것.
			double percentage = curFileSize * 100LL / (totalFileSize + (double)1e-9);
			printf("(%.2lf%%)", percentage);
		}
		puts("");
		*pastSecond = now;
		*elapseFileSize = 0LL;
	}
}

// return: fileSize
long long SendFileToServer(bool& isTCP, const char *filename, SOCKET& sock, sockaddr_in& echoAddr, int addrSize) {
	ifstream file(filename, ios::in | ios::binary);
	if (!file.is_open()) {
		fprintf(stderr, "File is not exists: \"%s\"\n", filename);
		return -1;
	}

	long long realFilesize = file.tellg();
	file.seekg(0, ios::end);
	realFilesize = file.tellg() - realFilesize;
	printf("\nSendFileToServer: %s\n", filename);
	printf("Estimated File Size: %lld bytes\n", realFilesize);

	// 파일의 크기가 설정한 크기 한도(UDP_FILESIZE_LIMIT) 를 넘어가면, TCP로 변경한다.
	bool switchedProtocol = false;
	char servIP[STRING_LENGTH] = {};
	sprintf(servIP, "%d.%d.%d.%d",
		echoAddr.sin_addr.S_un.S_un_b.s_b1,
		echoAddr.sin_addr.S_un.S_un_b.s_b2,
		echoAddr.sin_addr.S_un.S_un_b.s_b3,
		echoAddr.sin_addr.S_un.S_un_b.s_b4
	);
	if (realFilesize >= UDP_FILESIZE_LIMIT) {
		ClientRequestSwitchProtocol(&isTCP, sock, echoAddr, PORT_NUMBER, servIP);
		switchedProtocol = true;
	}

	Sleep(100);

	char buffer[BUFFER_SIZE] = {};
	sprintf(buffer, "<send-file>%s%c", filename, 0);
	ClientSendToServer(isTCP, sock, buffer, BUFFER_SIZE, echoAddr, addrSize);

	Sleep(100);

	ZeroMemory(buffer, BUFFER_SIZE);
	sprintf(buffer, "<file-size>%lld...%c", realFilesize, 0);
	ClientSendToServer(isTCP, sock, buffer, BUFFER_SIZE, echoAddr, addrSize);

	Sleep(100);

	int recvMsgSize = ClientReceiveFromServer(isTCP, sock, buffer, BUFFER_SIZE, echoAddr, &addrSize);
	long long resendSize = getResendFileSize(buffer, recvMsgSize);

	Sleep(100);
	
	// 무결성 검사 시작
	ZeroMemory(buffer, BUFFER_SIZE);
	HASH_STR hash;

	file.seekg(0, ios::beg);
	streampos zeroOffset = file.tellg();
	file.seekg(resendSize, ios::beg);
	streampos startOffset = file.tellg();

	long long calculatedFileSize = startOffset - zeroOffset;
	long long elapseFileSize = 0LL;
	time_t lastTickTime = clock();
	bool isEof = false;
	do {
		streampos prevPos = file.tellg();
		ZeroMemory(buffer, BUFFER_SIZE);
		file.read((char *)&buffer, BUFFER_SIZE);
		int recvBlockSize = 0;
		isEof = file.eof();
		// 남은 블럭의 크기가 BUFFER_SIZE보다 큰 경우, 정확한 크기로 파일에 기록한다.
		if (isEof) {
			file.clear();
			file.seekg(0, ios::end);
			long long realRestSize = file.tellg() - prevPos;

			file.clear();
			file.seekg(-realRestSize, ios::end);
			ZeroMemory(buffer, BUFFER_SIZE);
			file.read((char *)&buffer, realRestSize);
			recvBlockSize = ClientSendToServer(isTCP, sock, buffer, realRestSize, echoAddr, addrSize);
		}
		else {
			recvBlockSize = ClientSendToServer(isTCP, sock, buffer, BUFFER_SIZE, echoAddr, addrSize);
		}

		hash = md5(hash + buffer);
		calculatedFileSize += recvBlockSize;
		elapseFileSize += recvBlockSize;
		checkSpeedAndPercentage(filename, &elapseFileSize, calculatedFileSize, &lastTickTime, realFilesize);

		Sleep(5);
	} while(file.tellg() >= 0 && isEof == false);
	file.close();

	Sleep(100);
	
	// 무결성 검사를 위한 해쉬 결과를 함께 전송한다.
	sprintf(buffer, "</send-file>%s\0", hash.c_str());
	ClientSendToServer(isTCP, sock, buffer, strlen(buffer), echoAddr, addrSize);
	
	if (switchedProtocol) {
		Sleep(100);
		ClientRequestSwitchProtocol(&isTCP, sock, echoAddr, PORT_NUMBER, servIP);
	}

	return calculatedFileSize;
}

// 클라이언트가 보낸 첫 신호(<send-file>)를 기준으로 블럭 단위로 서버에서 저장한다.
long long SaveFileToServer(bool& isTCP, char *buffer, SOCKET& sock, SOCKET& clientSock, sockaddr_in& echoAddr, int* addrSize) {
	char filename[STRING_LENGTH] = {};
	long long ret = -1LL;
	if (isBeginToSendFile(buffer, filename)) {
		// <send-file> 이후 <file-size>
		ServerReceiveFromClient(isTCP, sock, buffer, BUFFER_SIZE, clientSock, echoAddr, addrSize);
		long long realFilesize = getFileSizeFromBuffer(buffer);

		if (realFilesize <= 0) {
			// <file-size>가 빨리 도착해서 파일 이름에 들어갔을 수 있다.
			int nameLen = strlen(filename);
			for (int i = 0; i < nameLen; ++i) {
				if (filename[i] == '<' || filename[i] == '>') {
					char zeroBuffer[STRING_LENGTH] = {};
					strncpy(filename + i, zeroBuffer, nameLen - i);
					break;
				}
			}
		}

		// 무결성 검사 준비
		HASH_STR hash;

		long long calculatedFileSize = 0LL;

		// 파일이 이미 있다면 이어하기를 시도한다.
		ifstream fileChk(filename, ios::in | ios::binary);
		if (fileChk.is_open()) {
			fileChk.seekg(0, ios::beg);
			streampos zeroOffset = fileChk.tellg();
			fileChk.seekg(0, ios::end);
			calculatedFileSize = fileChk.tellg() - zeroOffset;
			printf("File already exists... try re-receive from - %lld byte offset\n", calculatedFileSize);
			fileChk.close();
		}

		// <send-file> 이후 <retry-from>를 클라이언트에게 보냄
		ZeroMemory(buffer, BUFFER_SIZE);
		sprintf(buffer, "<retry-from>%lld", calculatedFileSize);
		int recvSize = ServerSendToClient(isTCP, sock, buffer, BUFFER_SIZE, clientSock, echoAddr, *addrSize);

		printf("\nProcessing File with \"%s\" with (Estimated: %lld bytes)\n", filename, realFilesize);
		ofstream file;
		if (calculatedFileSize > 0LL) {
			file.open(filename, ios::out | ios::app | ios::binary);
		}
		else {
			file.open(filename, ios::out | ios::binary);
		}
		int recvBlockSize = 0;
		time_t lastTickTime = clock();
		long long elapseFileSize = 0LL;
		do {
			ZeroMemory(buffer, BUFFER_SIZE);
			/* Block until receive message from a client */
			recvBlockSize = ServerReceiveFromClient(isTCP, sock, buffer, BUFFER_SIZE, clientSock, echoAddr, addrSize);
			// if (recvBlockSize < BUFFER_SIZE) printf("recvBlockSize = %d\n", recvBlockSize);
			if (recvBlockSize <= 0) break;
			if (isEndToSendFile(buffer)) break;

			hash = md5(hash + buffer);
			calculatedFileSize += recvBlockSize;
			elapseFileSize += recvBlockSize;
			checkSpeedAndPercentage(filename, &elapseFileSize, calculatedFileSize, &lastTickTime, realFilesize);
			file.write(buffer, recvBlockSize);
		} while (recvBlockSize > 0);
		file.close();

		string clientHash = extractHash(buffer);

		// 무결성 검사 종료 및 출력
		bool correctHash = compare_hash(clientHash, hash);
		printf("Compare Data Integrity..\n");
		printf("client: [%s]\nserver: [%s] .... %s\n", clientHash.c_str(), hash.c_str(), correctHash ? "CORRECT" : "FAIL");

		// 기다리고 있을 클라이언트에게 (아무 메시지나 가능하지만) 종료 메시지를 보냄
		ZeroMemory(buffer, sizeof(buffer));
		sprintf(buffer, "%s to File Transfer: \"%s\"\n", correctHash ? "Successed" : "Failed", filename);
		ServerSendToClient(isTCP, sock, buffer, strlen(buffer), clientSock, echoAddr, *addrSize);

		ret = calculatedFileSize;
	}

	return ret;
}

long long SendDirectoryToServer(bool isTCP, char *pathname, SOCKET& sock, sockaddr_in& echoAddr, int addrSize) {
	vector<FilePath> fileList;
	string root = getRealPath(pathname);
	getDirectoryList(root, &fileList);
	int listLength = fileList.size();
	string prefix(pathname); prefix.append("\\");

	// create sub directories
	set<string> subDirs;
	for (int i = 0; i < listLength; ++i) {
		FilePath& f = fileList[i];
		string relativePath = prefix + f.directory.substr(root.length());
		if (subDirs.find(relativePath) == subDirs.end()) {
			subDirs.insert(relativePath);
		}
	}

	for (set<string>::iterator it = subDirs.begin(); it != subDirs.end(); ++it) {
		char dirName[BUFFER_SIZE] = {};
		sprintf(dirName, "<create-dir>%s", it->c_str());
		ClientSendToServer(isTCP, sock, dirName, strlen(dirName), echoAddr, addrSize);
	}

	long long totalFilesSize = 0LL;

	// send files
	for (int i = 0; i < listLength; ++i) {
		FilePath& f = fileList[i];
		if (f.isDir) continue;
		string relativePath = prefix + f.directory.substr(root.length());
		totalFilesSize += SendFileToServer(isTCP, (relativePath + f.filename).data(), sock, echoAddr, addrSize);

		// 전송의 안정성을 위해 각 파일마다 전송의 딜레이를 둔다.
		Sleep(500);
	}

	return totalFilesSize;
}
