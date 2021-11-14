#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define SERVER_PORT 9000
#define CLIENT_MAX_NUMBER 2
#define RECV_SIZE 1024

#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

HANDLE my_print_event, my_recv_event;
int client_number = 0;

void err_quit(const char* msg);
void err_display(const char* msg);


DWORD WINAPI ProgressPrintProcess(LPVOID lpparameter) {
	while (true) {
		int result = WaitForSingleObject(my_print_event, INFINITE);
		if (result != WAIT_OBJECT_0) return 1;

		system("cls");
		EnterCriticalSection(&my_cs);
		for (auto it : my_threads_info) {
			int progress = it->progress;
			int limit = it->size;
			int percent = ((double)(progress) / (double)(limit)) * 100;

			cout << "스레드 " << it->index << " 수신률: " << percent << "% (" << progress << "/" << limit << ")\n";
		}
		LeaveCriticalSection(&my_cs);

		SetEvent(my_recv_event);
	}
	return 0;
}

DWORD WINAPI ClientReceiveProcess(LPVOID lpparameter) {
	SOCKET client_socket = reinterpret_cast<SOCKET>(lpparameter);

	while (true) {
		int buffer_length = 0;
		int result = recv(client_socket, (char*)(&buffer_length), sizeof(int), MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			err_display("receive 1");
			break;
		} else if (0 == result) {
			break;
		}

		char file_path[RECV_SIZE + 1];
		ZeroMemory(file_path, RECV_SIZE + 1);
		result = recv(client_socket, file_path, buffer_length, MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			err_display("receive 2");
			break;
		} else if (0 == result) {
			break;
		}

		result = recv(client_socket, (char*)(&buffer_length), sizeof(int), MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			err_display("receive 3");
			break;
		} else if (0 == result) {
			break;
		}

		ofstream file_writer(file_path, ios::binary);
		if (!file_writer) {
			cout << "파일 쓰기 오류" << '\n';
			break;
		}

		int progress = 0;
		char buffer[RECV_SIZE + 1];
		ZeroMemory(buffer, RECV_SIZE + 1);

		while (progress < buffer_length) {
			int result = WaitForSingleObject(my_recv_event, INFINITE);
			if (result != WAIT_OBJECT_0) return 1;

			result = recv(client_socket, buffer, RECV_SIZE, 0);

			if (SOCKET_ERROR == result) {
				err_display("receive 4");
				break;
			} else if (0 == result) {
				break;
			}

			progress += result;

			file_writer.write(buffer, result);
			int percent = ((double)(progress) / (double)(buffer_length)) * 100;

			cout << "클라이언트 " << client_socket << " 수신률: " << percent << "% (" << progress << "/" << buffer_length << ")\n";


			SetEvent(my_print_event);
		}
		if (0 == progress) break;

		file_writer.close();
	}

	closesocket(client_socket);
	return 0;
}

int main(void) {
	WSADATA wsa;

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listener) err_quit("socket");

	SOCKADDR_IN server_address;
	ZeroMemory(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);

	int result = bind(listener, (SOCKADDR*)(&server_address), sizeof(server_address));
	if (SOCKET_ERROR == result) err_quit("bind");

	result = listen(listener, CLIENT_MAX_NUMBER);
	if (SOCKET_ERROR == result) err_quit("listen");

	my_print_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!my_print_event) return 1;

	my_recv_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!my_recv_event) return 1;

	HANDLE print_thread = CreateThread(NULL, 0, ProgressPrintProcess, NULL, 0, NULL);
	if (!print_thread) return 1;

	cout << "서버 실행 중" << '\n';
	while (true) {
		SOCKADDR_IN client_address;
		int address_length = sizeof(client_address);
		ZeroMemory(&client_address, address_length);

		SOCKET client_socket = accept(listener, (SOCKADDR*)(&client_address), &address_length);
		if (SOCKET_ERROR == client_socket) {
			err_display("accept");
			continue;
		}

		cout << "클라이언트 접속 - IP 주소: " << inet_ntoa(client_address.sin_addr)
			<< ", 포트 번호: " << ntohs(client_address.sin_port) << '\n';

		HANDLE hthread = CreateThread(NULL, 0, ClientReceiveProcess, (LPVOID*)(&client_socket), 0, NULL);

		if (NULL == hthread) {
			closesocket(client_socket);
		} else {
			client_number++;
		}
	}

	closesocket(listener);

	CloseHandle(my_print_event);
	CloseHandle(my_recv_event);
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

	cout << msg << " - " << (char*)(lpMSGBuffer);

	LocalFree(lpMSGBuffer);
}
