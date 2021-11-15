#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <commctrl.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
using namespace std;

#include "Resource.h"

//#define SERVER_IP "172.30.1.11"
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define CLIENT_MAX_NUMBER 2
#define BUFF_SIZE 1024


SOCKET my_socket;

HWND my_filename_label;

OPENFILENAME my_open_dialog;
char file_path[BUFF_SIZE];
char file_name[BUFF_SIZE];
bool file_is_ready = false;

DWORD WINAPI ClientProcess(LPVOID lpparameter);
INT_PTR CALLBACK DlgProcedure(HWND, UINT, WPARAM, LPARAM);

void SendData(SOCKET socket, char* buffer, int length);
void ErrorAbort(const char* msg);
void ErrorDisplay(const char* msg);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR lpCmdLine,
					 _In_ int nCmdShow) {
	CreateThread(NULL, 0, ClientProcess, NULL, 0, NULL);
	
	ZeroMemory(&my_open_dialog, sizeof(LPOPENFILENAME));
	my_open_dialog.lStructSize = sizeof(my_open_dialog);
	my_open_dialog.lpstrFilter = "All\0*.*";
	my_open_dialog.nFilterIndex = 0;
	my_open_dialog.nMaxFile = BUFF_SIZE;
	my_open_dialog.nMaxFileTitle = BUFF_SIZE;
	my_open_dialog.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	DialogBox(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), NULL, DlgProcedure);

	closesocket(my_socket);
	WSACleanup();
	return 0;
}

INT_PTR CALLBACK DlgProcedure(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		case WM_INITDIALOG:
		{
			my_filename_label = GetDlgItem(hDlg, IDL_FILENAME);

			return TRUE;
		}

		case WM_COMMAND:
		{
			switch (wParam) {
				case IDC_FILEOPENBUTTON: // 파일 열기
				{
					ZeroMemory(file_path, sizeof(file_path));
					ZeroMemory(file_name, sizeof(file_name));
					my_open_dialog.hwndOwner = hDlg;

					my_open_dialog.lpstrFile = file_path;
					my_open_dialog.lpstrFileTitle = file_name;

					BOOL result = GetOpenFileName(&my_open_dialog);
					if (result) {
						file_is_ready = true;
						SetWindowText(my_filename_label, my_open_dialog.lpstrFileTitle);
					}
				}
				break;

				case IDC_SENDBUTTON:
				{
					if (file_is_ready) {
						long file_size = 0;
						char* file_buffer = nullptr;

						FILE* myfile = fopen(my_open_dialog.lpstrFileTitle, "rb");
						if (myfile) {
							fseek(myfile, 0, SEEK_END);
							file_size = ftell(myfile);
							fseek(myfile, 0, SEEK_SET);

							file_buffer = new char[file_size];
							ZeroMemory(file_buffer, file_size);
							fread(file_buffer, file_size, 1, myfile);
							fclose(myfile);
						} else {
							ErrorDisplay("fopen");
						}

						SendData(my_socket, file_name, strlen(file_name));
						SendData(my_socket, (char*)(file_buffer), file_size);
					}
				}
				break;

				case IDCLOSE:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			}
		}
		break;

		case WM_CLOSE:
		{
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

DWORD WINAPI ClientProcess(LPVOID lpparameter) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) return 1;

	my_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == my_socket) ErrorAbort("socket");

	SOCKADDR_IN server_address;
	int addr_size = sizeof(server_address);

	ZeroMemory(&server_address, addr_size);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_address.sin_port = htons(SERVER_PORT);

	if (SOCKET_ERROR == connect(my_socket, (SOCKADDR*)(&server_address), addr_size))
		ErrorAbort("connect");

	while (true) {

	}

	return 0;
}

void SendData(SOCKET socket, char* buffer, int length) {
	int result = send(socket, (char*)(&length), sizeof(int), 0);
	if (SOCKET_ERROR == result) {
		ErrorAbort("send 1");
	}

	result = send(socket, buffer, length, 0);
	if (SOCKET_ERROR == result) {
		ErrorAbort("send 2");
	}
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

	//cout << msg << " - " << (char*)(lpMSGBuffer);

	LocalFree(lpMSGBuffer);
}
