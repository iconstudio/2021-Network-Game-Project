#pragma comment(lib, "ws2_32")

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h> // ������ ���� API �ҷ�����


/// ������ ������ ����ϰ� ���α׷��� �����մϴ�.
void err_quit(const char* msg);

/// ������ ������ �ֿܼ� ����մϴ�.
void err_display(const char* msg);

/// ������ �̸��� IPv4 �ּҷ� ��ȯ�մϴ�.
BOOL GetIPAddress(const char* name, IN_ADDR* address);

/// IPv4 �ּҸ� ������ �̸����� ��ȯ�մϴ�.
BOOL GetDomainName(IN_ADDR address, char* name, int name_length);

int recvn(SOCKET sock, char* buffer, int length, int flags);

int main(void) {
	WSADATA wsa; // ������ ���� ����

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}
	//MessageBox(NULL, "���� �ʱ�ȭ ����", "�˸�", MB_OK);

	u_short x1 = 0x1234; // 4660
	u_long y1 = 0x12345678; // 305419896
	u_short x2;
	u_long y2;


	// ȣ��Ʈ ����Ʈ -> ��Ʈ��ũ ����Ʈ
	puts("[ȣ��Ʈ ����Ʈ -> ��Ʈ��ũ ����Ʈ]\n");
	x2 = htons(x1);
	y2 = htonl(y1);

	printf("0x%x -> 0x%x\n", x1, x2);
	printf("0x%x -> 0x%x\n", y1, y2);


	// ��Ʈ��ũ ����Ʈ -> ȣ��Ʈ ����Ʈ
	puts("[��Ʈ��ũ ����Ʈ -> ȣ��Ʈ ����Ʈ]\n");
	auto host_x2 = ntohs(x2);
	auto host_y2 = ntohl(y2);

	printf("0x%x -> 0x%x\n", x2, host_x2);
	printf("0x%x -> 0x%x\n", y2, host_y2);

	// �߸��� ��� ����
	puts("[�߸��� ��� ����]\n");
	printf("0x%x -> 0x%x\n", x1, htonl(x1));


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
