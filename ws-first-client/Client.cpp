#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
using namespace std;

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 512


void err_quit(const char* msg);

void err_display(const char* msg);

int recvn(SOCKET sock, char* buffer, int length, int flags);

int main(void) {
	int result;
	WSADATA wsa; // ������ ���� ����

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	SOCKET mysocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == mysocket) err_quit("socket");

	SOCKADDR_IN server_address;
	ZeroMemory(&server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
	server_address.sin_port = htons(SERVER_PORT);
	result = connect(mysocket, (SOCKADDR*)(&server_address), sizeof(server_address));
	if (SOCKET_ERROR == result) err_quit("connect");

	//
	//
	//*
	ifstream file_reader("kittylie.jpg", ios::binary);
	if (file_reader) {
		// seekg�� �̿��� ���� ũ�� ����
		file_reader.seekg(0, file_reader.end);
		int length = (int)file_reader.tellg();
		file_reader.seekg(0, file_reader.beg);

		// malloc���� �޸� �Ҵ�
		unsigned char * buffer = (unsigned char*)malloc(length);

		// read data as a block:
		file_reader.read((char*)buffer, length);
		file_reader.close();
		//*_data = buffer; 
		//*datalen = length;
	}
	//*/

	char buffer[BUFFER_SIZE];
	int buffer_length;
	const char* test_data[] = {
		"�ȳ��ϼ���"
		, "�ݰ�����"
		, "���õ��� �� �̾߱Ⱑ ���� �� ���׿�"
		, "���� �׷��׿�"
	};

	for (int i = 0; i < 4; i++) {
		buffer_length = strlen(test_data[i]);
		strncpy(buffer, test_data[i], buffer_length);

		//
		result = send(mysocket, (char*)(&buffer_length), sizeof(int), 0);
		if (SOCKET_ERROR == result) {
			err_display("send 1");
			break;
		}

		result = send(mysocket, buffer, buffer_length, 0);
		if (SOCKET_ERROR == result) {
			err_display("send 2");
			break;
		}

		printf("\n[TCP Ŭ���̾�Ʈ] %d+%d ����Ʈ�� ���½��ϴ�.\n", sizeof(int), result);
	}
	closesocket(mysocket);

	WSACleanup();

	return 0;
}

void err_quit(const char* msg) {
	LPVOID lpMSGBuffer; // �޽����� ������ ���� ���� �����Դϴ�.

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONERROR);

	// ���� ����
	LocalFree(lpMSGBuffer);

	// �ֿܼ� �ڵ� 1�� ����ϸ� ���α׷��� �����ϴ�.
	exit(1);
}

void err_display(const char* msg) {
	LPVOID lpMSGBuffer; // �޽����� ������ ���� ���� �����Դϴ�.

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	printf("[%s] %s", msg, (char*)lpMSGBuffer);

	// ���� ����
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
