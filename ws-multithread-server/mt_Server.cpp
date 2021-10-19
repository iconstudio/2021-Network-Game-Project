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

struct MyThread {
	SOCKET client_socket;
	HANDLE index;
	int progress = 0;
	int size = 1;
};

CRITICAL_SECTION my_cs;
HANDLE my_print_event, my_recv_event;
vector<MyThread*> my_threads_info;

void err_quit(const char* msg);
void err_display(const char* msg);

DWORD WINAPI print_processor(LPVOID lpparameter) {
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

DWORD WINAPI server_processor(LPVOID lpparameter) {
	MyThread* my_thread = (MyThread*)(lpparameter);
	SOCKET client_socket = my_thread->client_socket;

	while (true) {
		int buffer_length = 0;
		int result = recv(client_socket, (char*)(&buffer_length), sizeof(int), MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			err_display("receive 1");
			break;
		} else if (0 == result) {
			break;
		}

		char file_path[PATH_LENGTH];
		ZeroMemory(file_path, PATH_LENGTH);
		result = recv(client_socket, file_path, buffer_length, MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			err_display("receive 2");
			break;
		} else if (0 == result) {
			break;
		}

		// recv를 왜 쓰냐? recvn 쓰면 되는데?
		result = recv(client_socket, (char*)(&buffer_length), sizeof(int), MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			err_display("receive 3");
			break;
		} else if (0 == result) {
			break;
		}

		/*
			벡터를 안 쓰면 안되냐? 공유 자원 왜 쓰지?
			동적 할당은 파일의 정보를 모를 때 쓴다!!!!
			벡터 쓸 이유가 있나?

			코드가 너무 난잡한데?
		*/
		EnterCriticalSection(&my_cs);
		my_threads_info.push_back(my_thread);
		my_thread->size = buffer_length;
		LeaveCriticalSection(&my_cs);

		ofstream file_writer(file_path, ios::binary);
		if (!file_writer) {
			cout << "파일 쓰기 오류" << '\n';
			break;
		}

		int progress = 0;
		char buffer[BUFFER_SIZE + 1];
		ZeroMemory(buffer, BUFFER_SIZE + 1);

		while (progress < buffer_length) {
			int result = WaitForSingleObject(my_recv_event, INFINITE);
			if (result != WAIT_OBJECT_0) return 1;

			/*
				recvn 쓰면 아래 임계 영역 안써도 되지 않음?
			*/
			result = recv(client_socket, buffer, BUFFER_SIZE, 0);

			if (SOCKET_ERROR == result) {
				err_display("receive 4");
				break;
			} else if (0 == result) {
				break;
			}

			progress += result;
			EnterCriticalSection(&my_cs);
			my_thread->progress = progress;
			LeaveCriticalSection(&my_cs);
			file_writer.write(buffer, result);

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

	result = listen(listener, SOMAXCONN);
	if (SOCKET_ERROR == result) err_quit("listen");

	my_print_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (!my_print_event) return 1;
	my_recv_event = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (!my_recv_event) return 1;
	HANDLE print_thread = CreateThread(NULL, 0, print_processor, NULL, 0, NULL);
	if (!print_thread) return 1;

	my_threads_info.reserve(THREADS_NUM);
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

		cout << "클라이언트 접속 - IP 주소: " << inet_ntoa(client_address.sin_addr)
			<< ", 포트 번호: " << ntohs(client_address.sin_port) << '\n';

		MyThread* threadbox = new MyThread;
		threadbox->client_socket = client_socket;

		HANDLE hthread = CreateThread(NULL, 0, server_processor, threadbox, 0, NULL);
		threadbox->index = hthread;

		if (NULL == hthread) {
			delete threadbox;
			closesocket(client_socket);
		}
	}

	closesocket(listener);
	DeleteCriticalSection(&my_cs);
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
