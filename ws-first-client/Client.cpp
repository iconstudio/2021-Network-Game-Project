#pragma comment(lib, "ws2_32")
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <stdio.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define PATH_SIZE 512

void err_quit(const char* msg);

void err_display(const char* msg);

/*
	좀 더 간편하게 고정 길이 + 가변 길이를 구현하기 위해 사용자 정의 함수 선언
*/
int send_packet(SOCKET socket, char* buffer, int length, int flags) {
	/*
		처음에 데이터의 크기 전송, 이 자체의 크기는 sizeof(int)
		send: socket이 블로킹 소켓이고, 운영체제 송신 버퍼에 공간이 없으면..
	*/
	int result = send(socket, (char*)(&length), sizeof(int), flags);
	if (SOCKET_ERROR == result) {
		err_quit("send 1");
	}

	result = send(socket, buffer, length, 0);
	if (SOCKET_ERROR == result) {
		err_quit("send 2");
	}

	return result;
}

int main(void) {
	int result;
	WSADATA wsa;

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

	char file_path[PATH_SIZE];
	long file_size = 0;
	char* file_buffer = nullptr;

	printf("[TCP 클라이언트] 접속 IP 주소 - %s\n", SERVER_IP);
	printf("보낼 파일 이름> ");
	scanf_s("%s", &file_path, PATH_SIZE);
	file_path[PATH_SIZE - 1] = '\0';

	FILE* myfile = fopen(file_path, "rb");
	if (myfile) {
		fseek(myfile, 0, SEEK_END); // 파일 포인터 이동
		file_size = ftell(myfile); // 읽기 위치 반환
		fseek(myfile, 0, SEEK_SET);
		file_buffer = new char[file_size];
		ZeroMemory(file_buffer, file_size);

		fread(file_buffer, file_size, 1, myfile);

		fclose(myfile);
	} else {
		err_quit("fopen");
	}

	result = send_packet(mysocket, file_path, strlen(file_path), 0);
	printf("\n[TCP 클라이언트] 파일 이름으로 %d+%d 바이트를 보냈습니다.\n", sizeof(int), result);
	
	result = send_packet(mysocket, (char*)(file_buffer), file_size, 0);
	printf("\n[TCP 클라이언트] 파일 버퍼로 %d+%d 바이트를 보냈습니다.\n", sizeof(int), result);

	closesocket(mysocket);

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

	LocalFree(lpMSGBuffer);
}
