#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define SERVER_PORT 9000
#define INFO_LENGTH 1024
#define THREADS_MAX 64

#include <WinSock2.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
using namespace std;

struct MyThread {
	SOCKET client_socket;
	u_int index = 0;
	int progress = 0;
	int size = 1;
};

vector<MyThread*> my_threads;
int my_recv_count = 0;

HANDLE my_recv_event;

CRITICAL_SECTION my_cs;

void err_quit(const char* msg);
void err_display(const char* msg);

int receive_packet(SOCKET sock, char* buffer, int length) {
	int result, progress = 0;
	while (progress < length) {
		result = recv(sock, buffer + progress, length - progress, 0);
		if (SOCKET_ERROR == result) {
			return SOCKET_ERROR;
		} else if (0 == result) {
			break;
		}

		progress += result;
	}

	return progress;
}

void print_progress() {
	EnterCriticalSection(&my_cs);
	auto it = my_threads.cbegin();
	auto itend = my_threads.cend();
	LeaveCriticalSection(&my_cs);

	system("cls");
	for (; it != itend; ++it) {
		auto my_thread = *(it);
		int progress = my_thread->progress;
		int limit = my_thread->size;
		int percent = ((double)(progress) / (double)(limit)) * 100;

		cout << "스레드 " << my_thread->index << " 수신률: " << percent << "% (" << progress << "/" << limit << ")\n";
	}
}

DWORD WINAPI print_thread(LPVOID lpparameter) {
	return 0;
}

DWORD WINAPI server_thread(LPVOID lpparameter) {
	MyThread* my_thread = (MyThread*)(lpparameter);
	SOCKET client_socket = my_thread->client_socket;

	int result;
	while (true) {
		int buffer_length = 0;

		result = receive_packet(client_socket, (char*)(&buffer_length), sizeof(int));
		if (SOCKET_ERROR == result) {
			err_display("receive 1");
			break;
		} else if (0 == result) {
			break;
		}

		char* file_path = new char[buffer_length + 1];
		ZeroMemory(file_path, buffer_length + 1);
		result = receive_packet(client_socket, file_path, buffer_length);
		if (SOCKET_ERROR == result) {
			err_display("receive 2");
			break;
		} else if (0 == result) {
			break;
		}

		result = receive_packet(client_socket, (char*)(&buffer_length), sizeof(int));
		if (SOCKET_ERROR == result) {
			err_display("receive 3");
			break;
		} else if (0 == result) {
			break;
		}

		//
		char* file_buffer = new char[buffer_length + 1];
		ZeroMemory(file_buffer, buffer_length + 1);

		int progress = 0;
		while (progress < buffer_length) {
			result = WaitForSingleObject(my_recv_event, INFINITE);
			if (result != WAIT_OBJECT_0) return 1;

			result = recv(client_socket, file_buffer + progress, buffer_length - progress, 0);
			if (SOCKET_ERROR == result) {
				err_display("receive 4");
				break;
			} else if (0 == result) {
				break;
			}

			progress += result;
			my_thread->progress = progress;
			my_thread->size = buffer_length;
			print_progress();
		}
		if (0 == progress) break;

		if (file_path && file_buffer) {
			std::ofstream file_writer(file_path, ios::binary);

			if (file_writer) {
				file_writer.write(file_buffer, buffer_length);
				file_writer.close();
			} else {
				cout << "파일 쓰기 오류" << '\n';
				break;
			}
		}
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

	result = listen(listener, SOMAXCONN);
	if (SOCKET_ERROR == result) err_quit("listen");

	my_recv_event = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (!my_recv_event) return 1;

	my_threads.reserve(THREADS_MAX);
	InitializeCriticalSection(&my_cs);

	cout << "서버 실행 중" << '\n';
	while (true) {
		SOCKADDR_IN client_address;
		int address_length = sizeof(client_address);
		SOCKET client_socket = accept(listener, (SOCKADDR*)(&client_address), &address_length);
		if (SOCKET_ERROR == client_socket) {
			err_display("accept");
			break;
		}

		cout << "클라이언트 접속 - IP 주소: "  << inet_ntoa(client_address.sin_addr)
			<< ", 포트 번호: " << ntohs(client_address.sin_port) << '\n';
		
		MyThread* my_thread = new MyThread;
		my_thread->client_socket = client_socket;

		HANDLE hthread = CreateThread(NULL, 0, server_thread, my_thread, 0, NULL);
		my_thread->index = (u_int)hthread;

		if (hthread) {
			EnterCriticalSection(&my_cs);
			my_threads.push_back(my_thread);
			LeaveCriticalSection(&my_cs);
			CloseHandle(hthread);
		} else {
			delete my_thread;
			closesocket(client_socket);
		}
	}

	closesocket(listener);
	DeleteCriticalSection(&my_cs);
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

	// 버퍼 해제
	LocalFree(lpMSGBuffer);
}

