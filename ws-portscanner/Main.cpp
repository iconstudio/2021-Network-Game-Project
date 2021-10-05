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
	����ڷκ��� �ּ� ������ �޾� ������ �ּҿ��� ���� �ִ� ��Ʈ ��ȣ�� Ȯ���ϴ� ���α׷��Դϴ�.
*/
int main(void) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	char ipv4_address[ADDRESS_SIZE];
	ZeroMemory(&ipv4_address, sizeof(ipv4_address)); // �޸� ���� ���� �� ���ֱ�

	printf("������ �ּ�> ");
	scanf("%s", &ipv4_address);
	if (ipv4_address[ADDRESS_SIZE - 1] != '\0') // ������ ���ڸ� 0���� ���� ���ڿ��� �����
		ipv4_address[ADDRESS_SIZE - 1] = '\0';

	u_short port_min, port_max;

	printf("��Ʈ�� �ּڰ�> ");
	scanf("%hu", &port_min); // unsigned short �ޱ�

	printf("��Ʈ�� �ִ�> ");
	scanf("%hu", &port_max);
	puts("");

	if (port_max < port_min) port_min = port_max; // ���� Ȯ���ϱ�

	int result;
	u_long ip_number = inet_addr(ipv4_address); // ��Ʈ��ũ ����Ʈ ����. ���������� ���� �ּҸ� ����.
	u_short port_number;

	SOCKADDR_IN remote_address; // ���� �ּ�
	SOCKET mysock;

	// ��Ʈ ��ȸ
	for (u_short i = port_min; i <= port_max; ++i) {
		port_number = htons(i);

		ZeroMemory(&remote_address, sizeof(remote_address)); // �ּ� ����ü ���� �޸� �ʱ�ȭ
		remote_address.sin_family = AF_INET;
		remote_address.sin_addr.s_addr = ip_number; // ������ ���� ip �ּ�
		remote_address.sin_port = port_number;
		// closesocket�� �ϰ�, sin_zero[8] ���̶� ��� zeromemory�� ���� ����

		mysock = socket(AF_INET, SOCK_STREAM, 0); // tcp ����
		if (INVALID_SOCKET == mysock) err_quit("socket()");

		//err_quit("connect()");
		// ���� �Լ��� ���ŷ �Լ�
		// ������ �����߰ų�, �����ؼ� ������ ���� ��ȯ�� �Ѵ�. �����ϸ� 0. �������� ��� ���.
		// ��� ������ listening ���̾�� �Ѵ�. (������ ����� ��ٸ���.)
		// 
		// ������ ���� ���� ����. ����: ��Ʈ�� ȣ��Ʈ���� ��Ʈ��ũ ���������� �̿��ϴ� ���ø����̼�, ���μ����� �����ϱ� ���� ���� �����̴�. �׸��� �ü�� ���� ������ ���ᵵ �ǹ��Ѵ�.
		// ���� �õ��� �� tcp �ܿ��� 3���� ��Ŷ ��ȯ�� �Ͼ��.
		// 
		// �̶� ������ �߽� �ּҴ� �ü���� �˾Ƽ� ����. �����ʹ� �޸� bind�� �ʿ����.
		// ���� ����ü�� ũ�Ⱑ 16���� Ŀ�� �ݵ�� �ּҸ� ����Ѵ�. ipv6�� 28, ���ܼ��� 32, ������ 30
		// ���� ����ü�� Ÿ�� ĳ���� �ϴ� ������ ���� �Ϲ����� ������ �ּ� ����ü�� ���� �����̴�.
		result = connect(mysock, (SOCKADDR*)(&remote_address), sizeof(remote_address));
		if (SOCKET_ERROR != result) {
			printf("���ᰡ���� ��Ʈ: %u\n", ntohs(port_number));
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

	// ���� ����
	LocalFree(lpMSGBuffer);

	exit(1);
}

void err_display(const char* msg) {
	LPVOID lpMSGBuffer;

	int error_code = WSAGetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	printf("[%s] %s", msg, (char*)lpMSGBuffer);

	// ���� ����
	LocalFree(lpMSGBuffer);
}
