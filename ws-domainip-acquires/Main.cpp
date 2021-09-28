#pragma comment(lib, "ws2_32")

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h> // 윈도우 소켓 API 불러오기


/// 오류의 내용을 출력하고 프로그램을 종료합니다.
void err_quit(const char* msg);

/// 오류의 내용을 콘솔에 출력합니다.
void err_display(const char* msg);

/// 도메인 이름을 IPv4 주소로 변환합니다.
BOOL GetIPAddress(const char* name, IN_ADDR* address);

/// IPv4 주소를 도메인 이름으로 변환합니다.
BOOL GetDomainName(IN_ADDR address, char* name, int name_length);

int recvn(SOCKET sock, char* buffer, int length, int flags);

int main(void) {
	WSADATA wsa; // 윈도우 소켓 정보

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}
	//MessageBox(NULL, "윈속 초기화 성공", "알림", MB_OK);

	u_short x1 = 0x1234; // 4660
	u_long y1 = 0x12345678; // 305419896
	u_short x2;
	u_long y2;


	// 호스트 바이트 -> 네트워크 바이트
	puts("[호스트 바이트 -> 네트워크 바이트]\n");
	x2 = htons(x1);
	y2 = htonl(y1);

	printf("0x%x -> 0x%x\n", x1, x2);
	printf("0x%x -> 0x%x\n", y1, y2);


	// 네트워크 바이트 -> 호스트 바이트
	puts("[네트워크 바이트 -> 호스트 바이트]\n");
	auto host_x2 = ntohs(x2);
	auto host_y2 = ntohl(y2);

	printf("0x%x -> 0x%x\n", x2, host_x2);
	printf("0x%x -> 0x%x\n", y2, host_y2);

	// 잘못된 사용 예시
	puts("[잘못된 사용 예시]\n");
	printf("0x%x -> 0x%x\n", x1, htonl(x1));


	WSACleanup();

	return 0;
}

void err_quit(const char* msg) {
	LPVOID lpMSGBuffer; // 메시지의 내용을 담을 버퍼 변수입니다.

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONERROR);

	// 버퍼 해제
	LocalFree(lpMSGBuffer);

	// 콘솔에 코드 1을 출력하며 프로그램을 나갑니다.
	exit(1);
}

void err_display(const char* msg) {
	LPVOID lpMSGBuffer; // 메시지의 내용을 담을 버퍼 변수입니다.

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	printf("[%s] %s", msg, (char*)lpMSGBuffer);

	// 버퍼 해제
	LocalFree(lpMSGBuffer);
}

BOOL GetIPAddress(const char* name, IN_ADDR* address) {
	HOSTENT* ptr = gethostbyname(name);

	if (ptr == NULL) {
		err_display("gethostbyname()");
		return FALSE;
	}

	if (ptr->h_addrtype != AF_INET) {
		return FALSE;
	}

	memcpy(address, ptr->h_addr, ptr->h_length);
	return TRUE;
}

BOOL GetDomainName(IN_ADDR address, char* name, int name_length) {
	auto addr = (char*)(&address);

	auto size_address = sizeof(address);

	HOSTENT* ptr = gethostbyaddr(addr, size_address, AF_INET);

	if (ptr == NULL) {
		err_display("gethostbyaddr()");
		return FALSE;
	}

	if (ptr->h_addrtype != AF_INET) {
		return FALSE;
	}

	strncpy(name, ptr->h_name, name_length);
	return TRUE;
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
