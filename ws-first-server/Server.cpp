#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>

#define SERVER_PORT 9000
#define BUFFER_SIZE 512

void err_quit(const char* msg);

void err_display(const char* msg);

int recvn(SOCKET sock, char* buffer, int length, int flags);

int main(void) {
	int result;
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listener) err_quit("socket");

	SOCKADDR_IN server_address;
	ZeroMemory(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);
	result = bind(listener, (SOCKADDR*)(&server_address), sizeof(server_address));
	if (SOCKET_ERROR == result) err_quit("bind");

	result = listen(listener, SOMAXCONN); // 받을 수 있는 큐 크기
	if (SOCKET_ERROR == result) err_quit("listen");

	SOCKET client_socket;
	SOCKADDR_IN client_address;
	int address_length;

	char buffer[BUFFER_SIZE];
	int buffer_length;

	while (true) {
		address_length = sizeof(client_address);
		client_socket = accept(listener, (SOCKADDR*)(&client_address), &address_length);
		if (SOCKET_ERROR == result) {
			err_display("accept");
			break;
		}

		auto client_ipv4 = inet_ntoa(client_address.sin_addr);
		auto client_port = ntohs(client_address.sin_port);
		printf("\n[TCP 서버] 클라이언트 접속 - IP 주소: %s, 포트 번호 = %d\n"
			   , client_ipv4, client_port);

		int received;
		while (true) {
			received = recvn(client_socket, (char*)(&buffer_length), sizeof(int), 0);
			if (SOCKET_ERROR == received) {
				err_display("recvn 1");
				break;
			} else if (0 == received) {
				break;
			}

			received = recvn(client_socket, buffer, buffer_length, 0);
			if (SOCKET_ERROR == received) {
				err_display("recvn 2");
				break;
			} else if (0 == received) {
				break;
			}

			buffer[received] = '\0';
			printf("\n[TCP 서버] 받은 정보: %s\n", buffer);
		}

		printf("\n[TCP 서버] 클라이언트 종료 - IP 주소: %s, 포트 번호 = %d\n", client_ipv4, client_port);
		closesocket(client_socket);
	}
	closesocket(listener);

	WSACleanup();

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

	printf("[%s] %s", msg, (char*)lpMSGBuffer);

	// 버퍼 해제
	LocalFree(lpMSGBuffer);
}

int recvn(SOCKET sock, char* buffer, int length, int flags) {
	int received, left = length;
	char* ptr = buffer;

	while (0 < left) {
		received = recv(sock, ptr, left, flags);
		if (SOCKET_ERROR == received) {
			return SOCKET_ERROR;
		} else if (0 == received) {
			break;
		}

		left -= received;
		ptr += received;
	}

	return (length - left);
}
