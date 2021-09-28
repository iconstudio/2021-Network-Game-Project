/*
		Windows Socket 1.1: WinSock.dll (16��Ʈ), WSOCK32.dll (32��Ʈ)

		Windows Socket 1.1 Extenstion API: MSWSOCK.dll

		-> ���� API ��� ����
		Windows Socket 2.2: ws2_32.dll
*/
#pragma comment(lib, "ws2_32")
#include <WinSock2.h> // ������ ���� 2.2 API �ҷ�����
#include <stdio.h> // printf, puts


/*			= 0. ���� ó�� =

	err_quit(const char *msg);
	-> ������ ������ ����ϰ� ���α׷��� �����մϴ�.

	## ���� msg�� �ڷ����� char*�� �߾��µ� C++���� ������ �߻��ؼ� const char*�� �ٲپ����ϴ�.

	�� ����
		const char *msg: ��� ���� â�� ����

		LPVOID: �ƹ� ������ �ڷ��� (*void)
		LPCVOID: LPVOID�� �����
		LPTSTR: ��Ƽ����Ʈ���� TCHAR*, �����ڵ忡�� 16��Ʈ �����ڵ� ������ WCHAR�� ������ �ڷ��� (*WCHAR)
		LPCWSTR: LPTSTR�� �����
*/
void err_quit(const char* msg) {
	// �޽����� ������ ���� ���� �����Դϴ�.
	LPVOID lpMSGBuffer;

	// ���� ��ȣ�� �˾Ƴ��ϴ�. ������ ������ ���� �����Դϴ�.
	int error_code = WSAGetLastError();

	/*			= ���� ó�� =

		FormatMessage(DWORD flags, [LPCVOID source], DWORD message_id, DWORD language_id, WCHAR *buffer, DWORD size, args...);
		-> ���� �ڵ��� ���� ������ �˾Ƴ��� ���ؼ� ������ ���ۿ� �־��� ������ ������ ����ϴ�.

		�� ����
			DWORD flags: ������ ������ ������ �� ����� ���� (��Ʈ �÷���)
			LPCVOID source: ���� �� �޽��� ������ ��ġ (FORMAT_MESSAGE_FROM_HMODULE, FORMAT_MESSAGE_FROM_STRING)
			DWORD message_id: ���� �ڵ�
			DWORD language_id: ���� ������ ǥ���� ���
			WCHAR *buffer: ���� ������ ���� ���� ���� ������
			DWORD size: �����÷ο� ������ ���� ������ ũ��

		�� ����
			FORMAT_MESSAGE_ALLOCATE_BUFFER: ������ ������ ���� ������ �ڵ����� �Ҵ��Ѵ�.
			FORMAT_MESSAGE_FROM_SYSTEM: �ü������ ���� �޽����� �����´�.

			 �� �� �÷��׸� �����߱� ������, ���� ���Ǹ� ��� �������� �����ϴ� �ι�° ���ڸ� NULL�� �մϴ�.
			�׸��� ������° ������ ������ ũ�⵵ ������� 0���� �� �� �ֽ��ϴ�. ������ ���� ���ڵ� NULL�� �Ӵϴ�.

			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)�� � ü���� �⺻ �� �����ɴϴ�.

			 ����� ���� ������ LocalFree ������ �����ؾ� �մϴ�.
	*/
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	/*
		MessageBox([HWND handle], LPCWSTR content, LPCWSTR title UINT icon);

		�� ����
			ù ������ â �ڵ��� NULL�� �ָ� ���� â�� �ڵ��� �����ɴϴ�.
	*/
	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONERROR);

	LocalFree(lpMSGBuffer);

	// ������Ʈ�� �ڵ� 1�� ����ϸ� ���α׷��� �����ϴ�.
	exit(1);
}

int main(void) {
	/*			= 1. ���� �ʱ�ȭ =

		WSAStartup(WORD version, WSADATA* informations);
		-> ���ϵ��� ����� ������ ��û�ϰ� ������ ���� (ws2_32 ���̺귯��)�� �ʱ�ȭ�մϴ�.

		�� ����
			WORD version: ������ ������ �����Դϴ�. '1.1', '2.2'�� �ֽ��ϴ�.
			WSADATA* informations: �ü���� �����ִ� ���� ������ ���� �����Դϴ�.

		�� ����
			�� �Լ��� �����ϸ� ws2_32.dll�� �޸𸮿� �ö��� �ʽ��ϴ�. �̶��� WSAGetLastError��
			����Ȯ�� �ڵ带 �������Ƿ� ���� �������ϴ�. �׷��� �� �ʱ�ȭ �Լ��� ���� ���� �ڵ带
			��ȯ�մϴ�.
	*/
	WORD version_22 = MAKEWORD(2, 2);
	WORD version_11 = MAKEWORD(1, 1);
	WSADATA wsa; // ������ ���� ����

	if (WSAStartup(version_11, &wsa) != 0) {
		return 1;
	}
	MessageBox(NULL, "���� �ʱ�ȭ ����", "�˸�", MB_OK);

	///@ �������� 2-1 ���� ����
	puts("������ ����");
	printf("wVersion: %u \nwHighVersion: %u \nszDescription: %s \nszSystemStatus: %s \n"
		   , wsa.wVersion, wsa.wHighVersion, wsa.szDescription, wsa.szSystemStatus);


	/*			= 2. ���� ������ �ݱ� =

		SOCKET socket(int address_familiy, int socket_type, int net_protocol);
		-> �־��� ���ڿ� ���� ������ �����, ���� ��ũ���͸� ����� �� �ڵ��� ��ȯ�մϴ�.
		������ �̿��� ����ϴ� ������ ���� ���� ��������(��űԾ�)�� ����ؾ߸� �մϴ�.

		�� ����
			int address_familiy: ����� ��븦 �����ϰԲ� ������ �� �ִ� �ּҸ� ���ϴ� ���
			�ּ� ü��� ���������� ������ ���� �޶�����.

				* AF_INET: ���Ἲ TCP/SOCK_STREAM, �񿬰Ἲ UDP/SOCK_DGRAM, etc.
				* AF_INET6: ���Ἲ TCP, �񿬰Ἲ UDP, IPv6������ IPv6�� IPv4�� ȣȯ���� Ȯ��
				* AF_IrDA (�ٰŸ� ���ܼ� ���), AF_BTH (�������)


			int socket_type: ��űԾ��� Ư��

				* SOCK_STREAM: �ŷڼ� �ִ� ������ ���� ��� ����, ������ ��������
								���� ������ �߿���. �������� ����Ʈ ����� ����.
				* SOCK_DGRAM: �ŷڼ� ���� ������ ���� ��� ����, �񿬰��� ��������
								���� ������ ������� ���� ���� �ӵ� ����. �ѹ��� ���� �� �ִ�
								������ ũ�Ⱑ ���ѵ��ְ�, �� �����͵� ���̿� ��谡 �ִ�.

			* ����� �������� ����� ���� ���� (1)
			+-----------+-----------------------+---------------+
			| ��������  |       �ּ� ü��       |   ���� ����   |
			+-----------+-----------------------+---------------+
			| TCP       | AF_INET �Ǵ� AF_INET6 | SOCK_STREAM   |
			+-----------+-----------------------+---------------+
			| UDP       | AF_INET �Ǵ� AF_INET6 | SOCK_DGRAM    |
			+-----------+-----------------------+---------------+

			�ּ� ü�谡 ���Ƶ� ���� ������ �ٸ��� ������ �� �ִ�.


			int net_protocol: �������� ����

			* ����� �������� ����� ���� ���� (2)
			+-----------+-----------------------+---------------+-----------------+
			| ��������  |       �ּ� ü��       |   ���� ����   |  �������� ���  |
			+-----------+-----------------------+---------------+-----------------+
			| TCP       | AF_INET �Ǵ� AF_INET6 | SOCK_STREAM   | IPPROTO_TCP (0) |
			+-----------+-----------------------+---------------+-----------------+
			| UDP       | AF_INET �Ǵ� AF_INET6 | SOCK_DGRAM    | IPPROTO_UDP (0) |
			+-----------+-----------------------+---------------+-----------------+

			������ TCP�� UDP ���������� ù��°�� �ι�° ���ڸ� �������� �����ǹǷ�
			�밳 ������ ���ڴ� 0�� ����ִ´�.
	*/


	// TCP ������ �����մϴ�.
	SOCKET tcp_socket = socket(
		AF_INET,			// �Ϲ� �ּ� ü��
		SOCK_STREAM,		// ���Ἲ
		0					// ������ �ʿ䰡 ��� 0
	);


	if (tcp_socket == INVALID_SOCKET) {
		err_quit("socket()");
	}
	MessageBox(NULL, "TCP ���� ���� ����", "�˸�", MB_OK);


	/*			= 3. ���� �ݱ� =

		closesocket(SOCKET socket);
		-> ������ �ݰ� �޸𸮸� �����մϴ�.

		�� ����
			��ȯ���� 0 Ȥ�� ������ �� SOCKET_ERROR�Դϴ�.
	*/
	closesocket(tcp_socket);


	/*			= 4. ������ ���� ���� =

		WSACleanup();
		-> ���� ��� �ߴ��� �ü���� �˸��� ���̺귯���� �����մϴ�.

		�� ����
			�� �Լ��� �����ϸ� WSAGetLastError�� ������ �˾Ƴ� �� �ֽ��ϴ�.
			��ȯ���� 0 Ȥ�� ������ �� SOCKET_ERROR�Դϴ�.
	*/
	WSACleanup();

	return 0;
}
