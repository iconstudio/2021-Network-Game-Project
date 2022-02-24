#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SERVER_PORT 9000
#define PATH_LENGTH 1024
#define THREADS_NUM 2
#define BUFFER_SIZE 1024

#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

void SendData(SOCKET socket, char* buffer, int length);
void ErrorAbort(const char* msg);
void ErrorDisplay(const char* msg);

namespace Selector {
	struct SOCKETINFO {
		SOCKET sock;
		char buffer[BUFFER_SIZE + 1];

		int sz_recv;
		int sz_send;
	};

	int sockets_number = 0;
	auto socket_infos = new SOCKETINFO*[FD_SETSIZE];

	BOOL AddressSocket(SOCKET sock) {
		if (FD_SETSIZE <= sockets_number) {
			cerr << "Cannpt create a new socket.\n";
			return FALSE;
		}

		auto info = new SOCKETINFO();
		if (NULL == info) {
			cerr << "Memory size exceed.\n";
			return FALSE;
		}

		info->sock = sock;
		info->sz_recv = 0;
		info->sz_send = 0;
		ZeroMemory(info->buffer, BUFFER_SIZE);

		socket_infos[sockets_number++] = info;

		return TRUE;
	}

	void RemoveSocket(int index) {
		auto& info = socket_infos[index];

		cout << "[TCP]Client exits: " << info->sock << "\n";

		closesocket(info->sock);
		delete info;

		if (index != sockets_number - 1) {
			socket_infos[index] = socket_infos[sockets_number - 1];
		}
		sockets_number--;
	}
}

int main() {
	int result;
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listener) ErrorAbort("socket");

	SOCKADDR_IN server_address;
	ZeroMemory(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);

	result = bind(listener, (SOCKADDR*)(&server_address), sizeof(server_address));
	if (SOCKET_ERROR == result) ErrorAbort("bind");

	result = listen(listener, SOMAXCONN);
	if (SOCKET_ERROR == result) ErrorAbort("listen");

	auto ON = 1UL;
	result = ioctlsocket(listener, FIONBIO, &ON);
	if (SOCKET_ERROR == result) ErrorAbort("ioctlsocket");

	FD_SET wset, rset;
	SOCKET client_socket;
	SOCKADDR_IN client_address;
	int address_length, i;

	while (true) {
		FD_ZERO(&wset);
		FD_ZERO(&rset);
		FD_SET(listener, &rset);

		for (i = 0; i < Selector::sockets_number; ++i) {
			auto& info = Selector::socket_infos[i];
			if (info->sz_send < info->sz_recv) {
				FD_SET(info->sock, &wset);
			} else {
				FD_SET(info->sock, &rset);
			}
		}

		result = select(0, &rset, &wset, NULL, NULL); // 무한 대기
		if (SOCKET_ERROR == result) ErrorAbort("select");

		// #1
		if (FD_ISSET(listener, &rset)) {
			address_length = sizeof(client_address);
			client_socket = accept(listener, (SOCKADDR*)(&client_address), &address_length);
			if (INVALID_SOCKET == client_socket) {
				ErrorDisplay("select");
			} else {
				cout << "[TCP]Client is connected: " << inet_ntoa(client_address.sin_addr) << endl;
				Selector::AddressSocket(client_socket);
			}

		} // #1

		// #2
		for (i = 0; i < Selector::sockets_number; ++i) {
			auto& info = Selector::socket_infos[i];
			auto sock = info->sock;
			auto& buffer = info->buffer;

			if (FD_ISSET(sock, &rset)) {
				result = recv(sock, buffer, BUFFER_SIZE, 0);
				if (SOCKET_ERROR == result) {
					ErrorDisplay("recv");
					Selector::RemoveSocket(i);
					continue;
				}

				if (0 == result) {
					Selector::RemoveSocket(i);
					continue;
				}

				info->sz_recv = result;

				cout << "받은 정보: " << buffer << "\n";
			} else if (FD_ISSET(sock, &rset)) {
				auto& send_bytes = info->sz_send;
				auto& recv_bytes = info->sz_recv;

				result = send(sock, buffer + send_bytes, recv_bytes - send_bytes, 0);
				if (SOCKET_ERROR == result) {
					ErrorDisplay("send");
					Selector::RemoveSocket(i);
					continue;
				}

				send_bytes += result;
				if (recv_bytes == send_bytes) {
					recv_bytes = send_bytes = 0;
				}
			}
		} // #2
	}

	WSACleanup();
	return 0;
}


void ErrorAbort(const char* msg) {
	LPVOID lpMSGBuffer;

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONERROR);

	LocalFree(lpMSGBuffer);

	exit(1);
}

void ErrorDisplay(const char* msg) {
	LPVOID lpMSGBuffer;

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	cout << msg << " - " << (char*)(lpMSGBuffer);

	LocalFree(lpMSGBuffer);
}
