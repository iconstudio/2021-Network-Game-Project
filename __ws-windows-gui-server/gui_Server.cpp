#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <commctrl.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

#define SERVER_PORT 9000
#define CLIENT_MAX_NUMBER 2
#define RECV_SIZE 1024

#define IDL_CLIENT1 110
#define IDL_CLIENT2 111
#define IDL_NOCLIENT 112
#define IDL_CLIENT1_INFO 113
#define IDL_CLIENT2_INFO 114

#define IDC_CLOSE 1000
#define IDC_PROGRESS1 1001
#define IDC_PROGRESS2 1002

int client_number = 0;
SOCKET listener;

HINSTANCE my_hInstance;
HWND my_noclient_label; // 연결 알림 텍스트
HWND my_label[CLIENT_MAX_NUMBER];
HWND my_progress[CLIENT_MAX_NUMBER];
HWND my_receive_percent[CLIENT_MAX_NUMBER];
HANDLE my_recv_event;

int ReceiveFile(SOCKET sk, int client_index, char* data, int data_length, int flags);
DWORD WINAPI ServerProcess(LPVOID lpparameter);
DWORD WINAPI ReceiveFileThread(LPVOID lpparameter);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

INT_PTR CALLBACK DlgProcedure(HWND, UINT, WPARAM, LPARAM);

void ErrorAbort(const char* msg);
void ErrorDisplay(const char* msg);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow) {
	my_hInstance = hInstance;
	
	WNDCLASSEX properties;
	properties.cbSize = sizeof(WNDCLASSEX);
	properties.style = CS_HREDRAW | CS_VREDRAW;
	properties.lpfnWndProc = WndProc;
	properties.cbClsExtra = 0;
	properties.cbWndExtra = 0;
	properties.hInstance = hInstance;
	properties.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	properties.lpszMenuName = "Server";
	properties.lpszClassName = "MyGUIServer";
	if (!RegisterClassEx(&properties)) return 1;

	HWND hWnd = CreateWindow("MyGUIServer", "Server"
							 , WS_POPUP | WS_CAPTION | WS_MINIMIZEBOX
							 , CW_USEDEFAULT, 0, 640, 480, nullptr, nullptr
							 , hInstance, nullptr);
	if (!hWnd) return 1;

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	my_recv_event = CreateEvent(NULL, FALSE, TRUE, NULL);
	if (!my_recv_event) return 1;

	CreateThread(NULL, 0, ServerProcess, NULL, 0, NULL);

	MSG msg;
	while (true) {
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
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


	while (true) {
		SOCKADDR_IN client_address;
		int address_length = sizeof(client_address);
		ZeroMemory(&client_address, address_length);

		SOCKET client_socket = accept(listener, (SOCKADDR*)(&client_address), &address_length);
		if (SOCKET_ERROR == client_socket) {
			ErrorDisplay("accept");
			continue;
		}

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
		char infotext[32];
		ZeroMemory(infotext, 32);

		wsprintf(infotext, TEXT("진행도: %d%%"), percent);

		SetWindowText(my_receive_percent[client_index], infotext);

		Sleep(10);
		SetEvent(my_recv_event);
	}

	return (progress);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_CREATE:
		{
			my_progress[0] = CreateWindowEx(0, PROGRESS_CLASS, TEXT("")
											   , WS_CHILD | WS_VISIBLE
											   , 20, 60, 320, 64, hWnd, (HMENU)IDC_PROGRESS1
											   , my_hInstance, NULL);

			my_progress[1] = CreateWindowEx(0, PROGRESS_CLASS, TEXT("")
											   , WS_CHILD | WS_VISIBLE
											   , 20, 144, 320, 64, hWnd, (HMENU)IDC_PROGRESS2
											   , my_hInstance, NULL);

			CreateWindow(TEXT("button"), TEXT("Click Me"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			450, 396, 170, 64, hWnd, (HMENU)IDC_CLOSE, my_hInstance, NULL);

			// 처음에는 진행도 안보이기
			for (int i = 0; i < CLIENT_MAX_NUMBER; ++i) {
				ShowWindow(my_receive_percent[i], FALSE);
			}
		}
		break;



		case WM_PAINT:
		{

		}
		break;

		case WM_DESTROY:
		{
			for (int i = 0; i < CLIENT_MAX_NUMBER; ++i) {
				DestroyWindow(my_progress[i]);
			}
			
			closesocket(listener);
			CloseHandle(my_recv_event);
			WSACleanup(); 

			KillTimer(hWnd, 0);
			PostQuitMessage(0);
		}
		break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
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

	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONWARNING);

	LocalFree(lpMSGBuffer);
}
