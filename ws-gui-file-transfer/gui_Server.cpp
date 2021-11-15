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

#include "resource.h"


HANDLE my_threads[CLIENT_MAX_NUMBER];
HANDLE my_recv_event;
int client_number = 0;

int ReceiveFile(SOCKET sk, char* file_path, int data_length, int flags);
DWORD WINAPI ServerProcess(LPVOID lpparameter);
DWORD WINAPI ReceiveProcess(LPVOID lpparameter);
INT_PTR CALLBACK DlgProcedure(HWND, UINT, WPARAM, LPARAM);
void ErrorAbort(const char* msg);
void ErrorDisplay(const char* msg);

int APIENTRY WinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPSTR lpCmdLine,
					 _In_ int nCmdShow) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	SOCKET listener = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == listener) ErrorAbort("socket");

	SOCKADDR_IN server_address;
	ZeroMemory(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(SERVER_PORT);

	int result = bind(listener, (SOCKADDR*)(&server_address), sizeof(server_address));
	if (SOCKET_ERROR == result) ErrorAbort("bind");

	result = listen(listener, CLIENT_MAX_NUMBER);
	if (SOCKET_ERROR == result) ErrorAbort("listen");

	my_recv_event = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (!my_recv_event) return 1;

	//DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProcedure);

	cout << "서버 실행 중" << '\n';
	while (true) {
		SOCKADDR_IN client_address;
		int address_length = sizeof(client_address);
		ZeroMemory(&client_address, address_length);

		SOCKET client_socket = accept(listener, (SOCKADDR*)(&client_address), &address_length);
		if (SOCKET_ERROR == client_socket) {
			ErrorDisplay("accept");
			continue;
		}

		cout << "클라이언트 접속 - IP 주소: " << inet_ntoa(client_address.sin_addr)
			<< ", 포트 번호: " << ntohs(client_address.sin_port) << '\n';

		HANDLE hthread = CreateThread(NULL, 0, ReceiveProcess, (LPVOID)(client_socket), 0, NULL);

		if (NULL == hthread) {
			closesocket(client_socket);
		} else {
			if (client_number < CLIENT_MAX_NUMBER) {
				my_threads[client_number++] = hthread;
				break;
			}
		}
	}

	WaitForMultipleObjects(CLIENT_MAX_NUMBER, my_threads, TRUE, INFINITE);

	closesocket(listener);

	CloseHandle(my_recv_event);
	WSACleanup();
	return 0;
}

LRESULT CALLBACK DlgProcedure(HWND, UINT, WPARAM, LPARAM) {
	return TRUE;
}

DWORD WINAPI ServerProcess(LPVOID lpparameter) {
	//
	return 0;
}

DWORD WINAPI ReceiveProcess(LPVOID lpparameter) {
	SOCKET client_socket = (SOCKET)(lpparameter);

	while (true) {
		int buffer_length = 0;
		int result = recv(client_socket, (char*)(&buffer_length), sizeof(int), MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			ErrorDisplay("receive 1");
			break;
		} else if (0 == result) {
			break;
		}

		char file_path[RECV_SIZE];
		ZeroMemory(file_path, RECV_SIZE);

		result = recv(client_socket, file_path, buffer_length, MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			ErrorDisplay("receive 2");
			break;
		} else if (0 == result) {
			break;
		}

		result = recv(client_socket, (char*)(&buffer_length), sizeof(int), MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			ErrorDisplay("receive 3");
			break;
		} else if (0 == result) {
			break;
		}

		result = ReceiveFile(client_socket, file_path, buffer_length, 0);
		if (SOCKET_ERROR == result) {
			ErrorDisplay("receive 4");
			break;
		} else if (0 == result) {
			break;
		}
	}

	closesocket(client_socket);
	return 0;
}

int ReceiveFile(SOCKET sk, char* file_path, int data_length, int flags) {
	ofstream file_writer(file_path, ios::binary);
	if (!file_writer) {
		cerr << "파일 쓰기 오류" << endl;
		return 0;
	}

	int progress = 0;
	while (progress < data_length) {
		int result = WaitForSingleObject(my_recv_event, INFINITE);
		if (result != WAIT_OBJECT_0) return 0;

		char buffer[RECV_SIZE + 1];
		ZeroMemory(buffer, RECV_SIZE + 1);

		result = recv(sk, (buffer + progress), RECV_SIZE, flags);
		if (SOCKET_ERROR == result) {
			file_writer.close();
			return SOCKET_ERROR;
		} else if (0 == result) {
			file_writer.close();
			return 0;
		}

		progress += result;
		file_writer.write(buffer, result);

		int percent = ((double)(progress) / (double)(data_length)) * 100;
		cout << "클라이언트 " << sk << " 수신률: " << percent << "% (" << progress << "/" << data_length << ")\n";

		Sleep(10);
		SetEvent(my_recv_event);
	}

	file_writer.close();
	return (progress);
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
