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

/*
	프로그램의 길이를 더 짧게 만들 수 있다!!!!

	recvn 함수가 어떨 때 리턴을 하나. 원리가 무엇인가
	recv 함수가 읽은 데이터 크기만큼 반환을 해줍니다. 반복문에서 반환 값과
	읽은 길이를 뺄셈해서 얼마나 읽어야 할지 결정을 합니다.
*/

/*
	recv 함수를 그대로 쓰면 데이터를 한번에 다 받을 수 있다는 보장이 없기 때문에,
	인자로 데이터의 크기라는 확실한 기준을 주고 recv 함수를 여러번 실행시켜서 수신 버퍼에서
	데이터를 읽어옵니다.

	flag: 읽을 데이터의 종류 혹은 읽는 방법을 지정합니다.
	MSG_OOB: out of band(긴급데이터) 데이터를 읽습니다.
	MSG_PEEK: 메시지를 없애지 않고 읽기
	MSG_WAITALL: 버퍼 꽉 찰때 까지 대기하기
*/
int recvn(SOCKET sock, char* buffer, int length, int flags) {
	int received, left = length;
	char* ptr = buffer;

	int progress = 0;
	while (0 < left) {
		received = recv(sock, ptr, left, flags);
		if (SOCKET_ERROR == received) {
			return SOCKET_ERROR;
		} else if (0 == received) {
			break; // 클라이언트가 접속을 종료했다는 뜻이므로 받을 정보가 더 없어서 입니다.
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
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); // 자기 자신의 주소를 직접 넣는 대신 쓴다.
	server_address.sin_port = htons(SERVER_PORT);

	// 소켓에 서버의 주소 정보를 담아준다.
	int result = bind(listener, (SOCKADDR*)(&server_address), sizeof(server_address));
	if (SOCKET_ERROR == result) err_quit("bind");

	// 두번째 인자는 운영체제가 받을 수 있는 최대 메시지 갯수
	result = listen(listener, SOMAXCONN);
	if (SOCKET_ERROR == result) err_quit("listen");

	SOCKET client_socket;
	SOCKADDR_IN client_address;
	int address_length;

	puts("서버 실행 중");
	while (true) {
		address_length = sizeof(client_address);
		/*
			주소 구조체들의 크기가 너무 커서 전송하기엔 알맞지 않다.
			그래서 더 작고 범용적인 sockaddr 구조체로 변환한다.

			통신용 새로운 소켓 반환
		*/
		client_socket = accept(listener, (SOCKADDR*)(&client_address), &address_length);
		if (SOCKET_ERROR == client_socket) {
			err_display("accept");
			break;
		}

		char* client_ipv4 = inet_ntoa(client_address.sin_addr);
		u_short client_port = ntohs(client_address.sin_port);
		printf("\n[TCP 서버] 클라이언트 접속 - IP 주소: %s, 포트 번호: %d\n"
			   , client_ipv4, client_port);

		/*
			왜 교과서 처럼 버퍼 하나로 안썼냐.
			작은 파일은 문제가 없었는데,
			동영상 파일의 크기가 너무 커서 버퍼를 쓸 때 오류가 자꾸 생겼습니다.

			교수님: 이건 일반적인 문제가 아니다. 너만 생기는 문제같다!
			왜 이렇게 했지? 버퍼 길이를 왜 따로 받아?

			운영체제의 수신 버퍼를 읽어온다!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			recv 함수는 한번에 받아오지 않는다!!!!!!!!!!!!!!!!!!!!!!!!
		*/
		char* file_path = nullptr;
		int file_path_length = 0;
		char* file_buffer = nullptr;
		int file_buffer_length = 0;

		/*
			고정 + 가변 길이 데이터

			고정 데이터: 실제로 보낼 버퍼의 크기 (int, sizeof(int))
			가변 데이터: 버퍼 (sizeof(buffer))
		*/
		while (true) {
			// 파일 이름
			// (char*)(&file_path_length): 버퍼 안의 정보를 보내야 하는데 int라서 캐스팅을 복잡하게 한다.
			result = recvn(client_socket, (char*)(&file_path_length), sizeof(int), 0);
			if (SOCKET_ERROR == result) {
				err_display("recvn 1");
				break;
			} else if (0 == result) {
				break;
			}

			// 가변 길이: 파일 이름
			// 1바이트 널 문자 때문에 크기 + 1 해준다.
			file_path = new char[file_path_length + 1];
			result = recvn(client_socket, file_path, file_path_length, 0);
			if (SOCKET_ERROR == result) {
				err_display("recvn 2");
				break;
			} else if (0 == result) {
				break;
			}

			file_path[file_path_length] = '\0';

			// 고정 길이: 파일 버퍼 크기
			result = recvn(client_socket, (char*)(&file_buffer_length), sizeof(int), 0);
			if (SOCKET_ERROR == result) {
				err_display("recvn 3");
				break;
			} else if (0 == result) {
				break;
			}

			// 가변 길이: 파일 버퍼
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

		/*
			C언어에선 포인터가 널이 아니면 true
			처음에 recvn으로 읽었을 때, file_path가 읽혔으면 true
		*/
		if (file_path && file_buffer) {
			// 파일을 메모리에 올리고 디스크립터를 반환합니다.
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

