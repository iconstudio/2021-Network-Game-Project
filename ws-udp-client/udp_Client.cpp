#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 512

#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
using namespace std;

int main() {
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (INVALID_SOCKET == client_socket) err_quit("socket");

	SOCKADDR_IN server_address;
	ZeroMemory(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_address.sin_port = htons(SERVER_PORT);

	char buffer[BUFFER_SIZE + 1];

	while (true) {
		ZeroMemory(buffer, sizeof(buffer));

		int result = sendto(client_socket, buffer, BUFFER_SIZE, 0
							, (SOCKADDR*)(&server_address), sizeof(server_address));
		if (SOCKET_ERROR == result) {
			err_display("sendto");
			continue;
		}

		SOCKADDR_IN peer_address;
		int peer_length = sizeof(peer_address);

		ZeroMemory(&peer_address, peer_length);
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
		server_address.sin_port = htons(SERVER_PORT);

		result = recvfrom(client_socket, buffer, BUFFER_SIZE, 0
						  , (SOCKADDR*)(&peer_address), &peer_length);
		if (SOCKET_ERROR == result) {
			err_display("recvfrom");
			continue;
		}
	}

	closesocket(client_socket);

	return 0;
}

void err_quit(const char* msg) {
	LPVOID lpMSGBuffer;

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONERROR);

	LocalFree(lpMSGBuffer);

	exit(1);
}

void err_display(const char* msg) {
	LPVOID lpMSGBuffer;

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	cout << msg << " - " << (char*)(lpMSGBuffer);

	LocalFree(lpMSGBuffer);
}
