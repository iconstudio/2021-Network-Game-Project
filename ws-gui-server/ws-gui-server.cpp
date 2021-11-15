#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <commctrl.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "resource.h"

#define SERVER_PORT 9000
#define CLIENT_MAX_NUMBER 2
#define RECV_SIZE 1024


int client_number = 0;
SOCKET listener;

HWND my_noclient_label; // 연결 알림 텍스트
HWND my_label[CLIENT_MAX_NUMBER];
HWND my_progress[CLIENT_MAX_NUMBER];
HWND my_receive_percent[CLIENT_MAX_NUMBER];

HANDLE my_recv_event;

int ReceiveFile(SOCKET sk, int client_index, char* data, int data_length, int flags);

DWORD WINAPI ServerProcess(LPVOID lpparameter);
DWORD WINAPI ReceiveFileThread(LPVOID lpparameter);
INT_PTR CALLBACK DlgProcedure(HWND, UINT, WPARAM, LPARAM);
void ErrorAbort(const char* msg);
void ErrorDisplay(const char* msg);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow) {
	my_recv_event = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (!my_recv_event) return 1;

	CreateThread(NULL, 0, ServerProcess, NULL, 0, NULL);

	//CreateDialog()
	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProcedure);

	closesocket(listener);
	CloseHandle(my_recv_event);
	WSACleanup();
	return 0;
}

INT_PTR CALLBACK DlgProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_INITDIALOG:
		{
			my_noclient_label = GetDlgItem(hDlg, IDL_NOCLIENT);
			my_label[0] = GetDlgItem(hDlg, IDL_CLIENT1);
			my_progress[0] = GetDlgItem(hDlg, IDC_PROGRESS1);
			my_receive_percent[0] = GetDlgItem(hDlg, IDL_CLIENT1_INFO);
			my_label[1] = GetDlgItem(hDlg, IDL_CLIENT2);
			my_progress[1] = GetDlgItem(hDlg, IDC_PROGRESS2);
			my_receive_percent[1] = GetDlgItem(hDlg, IDL_CLIENT2_INFO);

			// 처음에는 진행도 안보이기
			for (int i = 0; i < CLIENT_MAX_NUMBER; ++i) {
				SendMessage(my_progress[i], PBM_SETRANGE, 0, MAKELPARAM(0, 100));

				ShowWindow(my_label[i], FALSE);
				ShowWindow(my_progress[i], FALSE);
				ShowWindow(my_receive_percent[i], FALSE);
			}

			return (INT_PTR)TRUE;
		}

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK) {
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
	}
	return (INT_PTR)FALSE;
}

DWORD WINAPI ServerProcess(LPVOID lpparameter) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	listener = socket(AF_INET, SOCK_STREAM, 0);
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

		HANDLE hthread = CreateThread(NULL, 0, ReceiveFileThread, (LPVOID)(client_socket), 0, NULL);

		if (NULL == hthread) {
			closesocket(client_socket);
		} else {
			if (0 == client_number) ShowWindow(my_noclient_label, FALSE);

			if (client_number < CLIENT_MAX_NUMBER) {
				ShowWindow(my_label[client_number], TRUE);
				ShowWindow(my_progress[client_number], TRUE);
				ShowWindow(my_receive_percent[client_number], TRUE);
				client_number++;
			}
		}
	}

	return 0;
}

DWORD WINAPI ReceiveFileThread(LPVOID lpparameter) {
	int client_index = client_number;
	SOCKET client_socket = (SOCKET)(lpparameter);

	while (true) {
		int buffer_length = 0;
		int result = recv(client_socket, (char*)(&buffer_length), sizeof(int), MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			ErrorDisplay("receive 1");
			continue;
		} else if (0 == result) {
			break;
		}

		char file_path[RECV_SIZE];
		ZeroMemory(file_path, RECV_SIZE);

		result = recv(client_socket, file_path, buffer_length, MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			ErrorDisplay("receive 2");
			continue;
		} else if (0 == result) {
			break;
		}

		result = recv(client_socket, (char*)(&buffer_length), sizeof(int), MSG_WAITALL);
		if (SOCKET_ERROR == result) {
			ErrorDisplay("receive 3");
			continue;
		} else if (0 == result) {
			break;
		}

		ofstream file_writer(file_path, ios::binary);
		if (!file_writer) {
			cerr << "파일 쓰기 오류" << endl;
			return 0;
		}

		char* file_buffer = new char[buffer_length];
		ZeroMemory(file_buffer, buffer_length);

		result = ReceiveFile(client_socket, client_index, file_buffer, buffer_length, 0);
		if (SOCKET_ERROR == result) {
			ErrorDisplay("receive 4");
			continue;
		} else if (0 == result) {
			break;
		}

		file_writer.write(file_buffer, result);
		file_writer.close();
	}

	closesocket(client_socket);
	return 0;
}

int ReceiveFile(SOCKET sk, int client_index, char* data, int data_length, int flags) {
	int progress = 0;
	while (progress < data_length) {
		int result = WaitForSingleObject(my_recv_event, INFINITE);
		if (result != WAIT_OBJECT_0) return 0;

		result = recv(sk, data + progress, RECV_SIZE, flags);

		if (SOCKET_ERROR == result) {
			return SOCKET_ERROR;
		} else if (0 == result) {
			return 0;
		}

		progress += result;

		int percent = ((double)(progress) / (double)(data_length)) * 100;
		SendMessage(my_progress[client_index], PBM_SETPOS, (WPARAM)(percent), 0);

		// 진행도 퍼센트
		char infotext[100];
		ZeroMemory(infotext, 100);
		sprintf(infotext, "진행도: %d%%", percent);

		SetWindowText(my_receive_percent[client_index], infotext);

		Sleep(10);
		SetEvent(my_recv_event);
	}

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
