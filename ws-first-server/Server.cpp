#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <stdio.h>

#define SERVER_PORT 9000
#define INFO_LENGTH 1024

void err_quit(const char* msg);
void err_display(const char* msg);

void print_receive(int progress, int size) {
	int percent = ((double)progress / (double)size) * 100;

	char info_text[INFO_LENGTH];
	sprintf_s(info_text, INFO_LENGTH, "데이터 수신 중: \n");

	char info_percentage[100];
	sprintf_s(info_percentage, 100, "전송률: %d%% (%d/%d)\n", percent, progress, size);
	strcat_s(info_text, info_percentage);

	system("cls");
	puts(info_text);
}

int recvn(SOCKET sock, char* buffer, int length, int flags) {
	int received, left = length;
	char* ptr = buffer;

	int progress = 0;
	while (0 < left) {
		received = recv(sock, ptr, left, flags);
		if (SOCKET_ERROR == received) {
			return SOCKET_ERROR;
		} else if (0 == received) {
			break;
		}

		progress += received;
		left -= received;
		ptr += received;
		print_receive(progress, length);
	}

	return (length - left);
}

int main(void) {
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

	int result = bind(listener, (SOCKADDR*)(&server_address), sizeof(server_address));
	if (SOCKET_ERROR == result) err_quit("bind");

	result = listen(listener, SOMAXCONN);
	if (SOCKET_ERROR == result) err_quit("listen");

	SOCKET client_socket;
	SOCKADDR_IN client_address;
	int address_length;

	puts("서버 실행 중");
	while (true) {
		address_length = sizeof(client_address);
		client_socket = accept(listener, (SOCKADDR*)(&client_address), &address_length);
		if (SOCKET_ERROR == client_socket) {
			err_display("accept");
			break;
		}

		char* client_ipv4 = inet_ntoa(client_address.sin_addr);
		u_short client_port = ntohs(client_address.sin_port);
		printf("\n[TCP 서버] 클라이언트 접속 - IP 주소: %s, 포트 번호: %d\n"
			   , client_ipv4, client_port);

		char* file_path = nullptr;
		int file_path_length = 0;
		char* file_buffer = nullptr;
		int file_buffer_length = 0;

		while (true) {
			// 파일 이름
			result = recvn(client_socket, (char*)(&file_path_length), sizeof(int), 0);
			if (SOCKET_ERROR == result) {
				err_display("recvn 1");
				break;
			} else if (0 == result) {
				break;
			}

			file_path = new char[file_path_length + 1];
			result = recvn(client_socket, file_path, file_path_length, 0);
			if (SOCKET_ERROR == result) {
				err_display("recvn 2");
				break;
			} else if (0 == result) {
				break;
			}

			file_path[file_path_length] = '\0';

			// 파일 내용
			result = recvn(client_socket, (char*)(&file_buffer_length), sizeof(int), 0);
			if (SOCKET_ERROR == result) {
				err_display("recvn 3");
				break;
			} else if (0 == result) {
				break;
			}

			file_buffer = new char[file_buffer_length + 1];
			result = recvn(client_socket, file_buffer, file_buffer_length, 0);
			if (SOCKET_ERROR == result) {
				err_display("recvn 4");
				break;
			} else if (0 == result) {
				break;
			}

			printf("\n[TCP 서버] 수신 완료 (크기: %d)\n", file_buffer_length);
		}

		printf("\n[TCP 서버] 클라이언트 종료 - IP 주소: %s, 포트 번호: %d\n", client_ipv4, client_port);

		if (file_path && file_buffer) {
			FILE* myfile = fopen(file_path, "wb");
			if (myfile) {
				fwrite(file_buffer, file_buffer_length, 1, myfile);
				fclose(myfile);
			} else {
				err_quit("fopen");
			}
		}

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

