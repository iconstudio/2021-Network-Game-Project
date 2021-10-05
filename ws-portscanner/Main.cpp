#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>

#define ADDRESS_SIZE 128
#define BUFFER_SIZE 512

void err_quit(const char* msg);

void err_display(const char* msg);

/*
	사용자로부터 주소 정보를 받아 아이피 주소에서 열려 있는 포트 번호를 확인하는 프로그램입니다.
*/
int main(void) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	char ipv4_address[ADDRESS_SIZE];
	ZeroMemory(&ipv4_address, sizeof(ipv4_address)); // 메모리 안의 더미 값 없애기

	printf("아이피 주소> ");
	scanf("%s", &ipv4_address);
	if (ipv4_address[ADDRESS_SIZE - 1] != '\0') // 마지막 글자를 0으로 만들어서 문자열로 만들기
		ipv4_address[ADDRESS_SIZE - 1] = '\0';

	u_short port_min, port_max;

	printf("포트의 최솟값> ");
	scanf("%hu", &port_min); // unsigned short 받기

	printf("포트의 최댓값> ");
	scanf("%hu", &port_max);
	puts("");

	if (port_max < port_min) port_min = port_max; // 범위 확인하기

	int result;
	u_long ip_number = inet_addr(ipv4_address); // 네트워크 바이트 정렬. 내부적으로 숫자 주소를 쓴다.
	u_short port_number;

	SOCKADDR_IN remote_address; // 원격 주소
	SOCKET mysock;

	// 포트 순회
	for (u_short i = port_min; i <= port_max; ++i) {
		port_number = htons(i);

		ZeroMemory(&remote_address, sizeof(remote_address)); // 주소 구조체 안의 메모리 초기화
		remote_address.sin_family = AF_INET;
		remote_address.sin_addr.s_addr = ip_number; // 연결할 서버 ip 주소
		remote_address.sin_port = port_number;
		// closesocket도 하고, sin_zero[8] 뿐이라 계속 zeromemory로 재사용 가능

		mysock = socket(AF_INET, SOCK_STREAM, 0); // tcp 소켓
		if (INVALID_SOCKET == mysock) err_quit("socket()");

		//err_quit("connect()");
		// 접속 함수는 블로킹 함수
		// 연결이 성공했거나, 실패해서 오류가 떠야 반환을 한다. 성공하면 0. 그전까진 계속 대기.
		// 대상 서버가 listening 중이어야 한다. (서버가 통신을 기다린다.)
		// 
		// 서버와 논리적 연결 설정. 이유: 포트가 호스트에서 네트워크 프로토콜을 이용하는 어플리케이션, 프로세스를 구분하기 위한 논리적 단위이다. 그리고 운영체제 단의 종단점 연결도 의미한다.
		// 연결 시도할 때 tcp 단에서 3개의 패킷 교환이 일어난다.
		// 
		// 이때 소켓의 발신 주소는 운영체제가 알아서 결정. 서버와는 달리 bind가 필요없다.
		// 소켓 구조체는 크기가 16으로 커서 반드시 주소를 사용한다. ipv6는 28, 적외선은 32, 블투는 30
		// 소켓 구조체를 타입 캐스팅 하는 이유는 가장 일반적인 형태의 주소 구조체를 쓰기 위함이다.
		result = connect(mysock, (SOCKADDR*)(&remote_address), sizeof(remote_address));
		if (SOCKET_ERROR != result) {
			printf("연결가능한 포트: %u\n", ntohs(port_number));
		}
		closesocket(mysock);
	}

	WSACleanup();

	return 0;
}

void err_quit(const char* msg) {
	LPVOID lpMSGBuffer;

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONERROR);

	// 버퍼 해제
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
