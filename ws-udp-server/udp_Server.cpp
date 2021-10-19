#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SERVER_PORT 9000
#define PATH_LENGTH 1024
#define BUFFER_SIZE 512
#define THREADS_NUM 2

#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

void err_quit(const char* msg);
void err_display(const char* msg);


int main(void) {
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET listener = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == listener) err_quit("socket");

	SOCKADDR_IN server_address;
	ZeroMemory(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);

	int result = bind(listener, (SOCKADDR*)(&server_address), sizeof(server_address));
	if (SOCKET_ERROR == result) err_quit("bind");

	result = listen(listener, SOMAXCONN);
	if (SOCKET_ERROR == result) err_quit("listen");

	SOCKADDR_IN client_addr;
	int address_length = 0;
	char buffer[BUFFER_SIZE + 1];

	while (true) {
		ZeroMemory(buffer, BUFFER_SIZE);

		address_length = sizeof(client_addr);
		result = recvfrom(listener, buffer, BUFFER_SIZE, 0
						  , (SOCKADDR*)(&client_addr), &address_length);
		if (SOCKET_ERROR == result) { // 0은 상관없음.
			err_display ("recvfrom");
			continue;
		}

		buffer[BUFFER_SIZE] = '\0';
		printf("수신 내용 (크기: %d): %s", result, buffer);

		// echo
		result = sendto(listener, buffer, result, 0
						, (SOCKADDR*)(&client_addr), sizeof(client_addr));
		if (SOCKET_ERROR == result) {
			err_display("sendto");
			continue;
		}
	}

	closesocket(listener);
}

