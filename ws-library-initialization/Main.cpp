#pragma comment(lib, "ws2_32")
#include <WinSock2.h> // ������ ���� 2.2 API �ҷ�����
#include <stdio.h>

void err_quit(const char* msg) {
	LPVOID lpMSGBuffer;

	int error_code = WSAGetLastError();

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONERROR);

	LocalFree(lpMSGBuffer);

	exit(1);
}

int main(void) {
	WSADATA wsa; // ������ ���� ����

	const WORD version_request = MAKEWORD(2, 0); // (�� ����, �� ����)
	if (WSAStartup(version_request, &wsa) != 0) {
		return 1;
	}

	printf("�ʱ�ȭ �� ����: %d.%d\n", LOBYTE(version_request), HIBYTE(version_request));
	BYTE highversion_sub = HIBYTE(wsa.wHighVersion);
	BYTE highversion_main = LOBYTE(wsa.wHighVersion);
	BYTE version_sub = HIBYTE(wsa.wVersion);
	BYTE version_main = LOBYTE(wsa.wVersion);

	printf("���� ���̺귯������ �����ϴ� ���� ���� ����(wHighVersion): %d.%d\n", highversion_main, highversion_sub);
	printf("���� ���� ���� ���� ���� (wVersion): %d.%d\n", version_main, version_sub);
	printf("���� ���̺귯�� ���� (szDescription): %s\n", wsa.szDescription);
	printf("���� ���� �Ǵ� ���̺귯�� ���� ���� (szSystemStatus): %s \n", wsa.szSystemStatus);

	MessageBox(NULL, "���� �ʱ�ȭ ����", "�˸�", MB_OK);

	WSACleanup();

	return 0;
}
