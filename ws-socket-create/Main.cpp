/*
		Windows Socket 1.1: WinSock.dll (16비트), WSOCK32.dll (32비트)

		Windows Socket 1.1 Extenstion API: MSWSOCK.dll

		-> 상위 API 모두 포함
		Windows Socket 2.2: ws2_32.dll
*/
#pragma comment(lib, "ws2_32")
#include <WinSock2.h> // 윈도우 소켓 2.2 API 불러오기
#include <stdio.h> // printf, puts


/*			= 0. 오류 처리 =

	err_quit(const char *msg);
	-> 오류의 내용을 출력하고 프로그램을 종료합니다.

	## 원래 msg의 자료형을 char*로 했었는데 C++에선 오류가 발생해서 const char*로 바꾸었습니다.

	※ 인자
		const char *msg: 띄울 오류 창의 제목

		LPVOID: 아무 포인터 자료형 (*void)
		LPCVOID: LPVOID의 상수형
		LPTSTR: 멀티바이트에선 TCHAR*, 유니코드에선 16비트 유니코드 문자인 WCHAR의 포인터 자료형 (*WCHAR)
		LPCWSTR: LPTSTR의 상수형
*/
void err_quit(const char* msg) {
	// 메시지의 내용을 담을 버퍼 변수입니다.
	LPVOID lpMSGBuffer;

	// 오류 번호를 알아냅니다. 오류의 종류에 따른 숫자입니다.
	int error_code = WSAGetLastError();

	/*			= 오류 처리 =

		FormatMessage(DWORD flags, [LPCVOID source], DWORD message_id, DWORD language_id, WCHAR *buffer, DWORD size, args...);
		-> 오류 코드의 실제 내용을 알아내기 위해서 지정한 버퍼에 주어진 오류의 정보를 담습니다.

		※ 인자
			DWORD flags: 오류의 내용을 가져올 때 사용할 동작 (비트 플래그)
			LPCVOID source: 갖고 올 메시지 정의의 위치 (FORMAT_MESSAGE_FROM_HMODULE, FORMAT_MESSAGE_FROM_STRING)
			DWORD message_id: 오류 코드
			DWORD language_id: 오류 내용을 표현할 언어
			WCHAR *buffer: 오류 내용을 담을 버퍼 변수 포인터
			DWORD size: 오버플로우 방지용 버퍼 변수의 크기

		※ 참고
			FORMAT_MESSAGE_ALLOCATE_BUFFER: 오류를 저장할 버퍼 공간을 자동으로 할당한다.
			FORMAT_MESSAGE_FROM_SYSTEM: 운영체제에서 오류 메시지를 가져온다.

			 이 두 플래그를 지정했기 때문에, 오류 정의를 어디서 가져올지 정의하는 두번째 인자를 NULL로 합니다.
			그리고 여섯번째 인자인 버퍼의 크기도 상관없이 0으로 둘 수 있습니다. 마지막 가변 인자도 NULL로 둡니다.

			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT)는 운영 체제의 기본 언어를 가져옵니다.

			 사용한 버퍼 변수는 LocalFree 등으로 해제해야 합니다.
	*/
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)(&lpMSGBuffer), 0, NULL);

	/*
		MessageBox([HWND handle], LPCWSTR content, LPCWSTR title UINT icon);

		※ 참고
			첫 인자인 창 핸들을 NULL로 주면 현재 창의 핸들을 가져옵니다.
	*/
	MessageBox(NULL, (LPTSTR)(lpMSGBuffer), msg, MB_ICONERROR);

	LocalFree(lpMSGBuffer);

	// 프롬포트에 코드 1을 출력하며 프로그램을 나갑니다.
	exit(1);
}

int main(void) {
	/*			= 1. 윈속 초기화 =

		WSAStartup(WORD version, WSADATA* informations);
		-> 소켓들이 사용할 버전을 요청하고 윈도우 소켓 (ws2_32 라이브러리)을 초기화합니다.

		※ 인자
			WORD version: 윈도우 소켓의 버전입니다. '1.1', '2.2'가 있습니다.
			WSADATA* informations: 운영체제가 돌려주는 소켓 구현에 대한 정보입니다.

		※ 설명
			이 함수가 실패하면 ws2_32.dll이 메모리에 올라가지 않습니다. 이때는 WSAGetLastError가
			부정확한 코드를 내뱉으므로 쓸모가 없어집니다. 그래서 이 초기화 함수가 직접 오류 코드를
			반환합니다.
	*/
	WORD version_22 = MAKEWORD(2, 2);
	WORD version_11 = MAKEWORD(1, 1);
	WSADATA wsa; // 윈도우 소켓 정보

	if (WSAStartup(version_11, &wsa) != 0) {
		return 1;
	}
	MessageBox(NULL, "윈속 초기화 성공", "알림", MB_OK);

	///@ 연습문제 2-1 과제 내용
	puts("윈도우 소켓");
	printf("wVersion: %u \nwHighVersion: %u \nszDescription: %s \nszSystemStatus: %s \n"
		   , wsa.wVersion, wsa.wHighVersion, wsa.szDescription, wsa.szSystemStatus);


	/*			= 2. 소켓 생성과 닫기 =

		SOCKET socket(int address_familiy, int socket_type, int net_protocol);
		-> 주어진 인자에 따라 소켓을 만들고, 소켓 디스크립터를 만들고 그 핸들을 반환합니다.
		소켓을 이용해 통신하는 양쪽이 서로 같은 프로토콜(통신규약)을 사용해야만 합니다.

		※ 인자
			int address_familiy: 통신할 상대를 유일하게끔 지정할 수 있는 주소를 정하는 방법
			주소 체계는 프로토콜의 종류에 따라 달라진다.

				* AF_INET: 연결성 TCP/SOCK_STREAM, 비연결성 UDP/SOCK_DGRAM, etc.
				* AF_INET6: 연결성 TCP, 비연결성 UDP, IPv6이지만 IPv6와 IPv4의 호환성도 확보
				* AF_IrDA (근거리 적외선 통신), AF_BTH (블루투스)


			int socket_type: 통신규약의 특성

				* SOCK_STREAM: 신뢰성 있는 데이터 전송 기능 제공, 연결형 프로토콜
								전송 순서가 중요함. 순차적인 바이트 기반의 전송.
				* SOCK_DGRAM: 신뢰성 없는 데이터 전송 기능 제공, 비연결형 프로토콜
								전송 순서는 상관없이 가장 빠른 속도 지향. 한번에 보낼 수 있는
								데이터 크기가 제한돼있고, 각 데이터들 사이에 경계가 있다.

			* 사용할 프로토콜 사용을 위한 설정 (1)
			+-----------+-----------------------+---------------+
			| 프로토콜  |       주소 체계       |   소켓 종류   |
			+-----------+-----------------------+---------------+
			| TCP       | AF_INET 또는 AF_INET6 | SOCK_STREAM   |
			+-----------+-----------------------+---------------+
			| UDP       | AF_INET 또는 AF_INET6 | SOCK_DGRAM    |
			+-----------+-----------------------+---------------+

			주소 체계가 같아도 소켓 종류를 다르게 설정할 수 있다.


			int net_protocol: 프로토콜 지정

			* 사용할 프로토콜 사용을 위한 설정 (2)
			+-----------+-----------------------+---------------+-----------------+
			| 프로토콜  |       주소 체계       |   소켓 종류   |  프로토콜 상수  |
			+-----------+-----------------------+---------------+-----------------+
			| TCP       | AF_INET 또는 AF_INET6 | SOCK_STREAM   | IPPROTO_TCP (0) |
			+-----------+-----------------------+---------------+-----------------+
			| UDP       | AF_INET 또는 AF_INET6 | SOCK_DGRAM    | IPPROTO_UDP (0) |
			+-----------+-----------------------+---------------+-----------------+

			하지만 TCP와 UDP 프로토콜은 첫번째와 두번째 인자만 정해져도 결정되므로
			대개 마지막 인자는 0을 집어넣는다.
	*/


	// TCP 소켓을 생성합니다.
	SOCKET tcp_socket = socket(
		AF_INET,			// 일반 주소 체계
		SOCK_STREAM,		// 연결성
		0					// 지정할 필요가 없어서 0
	);


	if (tcp_socket == INVALID_SOCKET) {
		err_quit("socket()");
	}
	MessageBox(NULL, "TCP 소켓 생성 성공", "알림", MB_OK);


	/*			= 3. 소켓 닫기 =

		closesocket(SOCKET socket);
		-> 소켓을 닫고 메모리를 해제합니다.

		※ 참고
			반환값은 0 혹은 실패할 시 SOCKET_ERROR입니다.
	*/
	closesocket(tcp_socket);


	/*			= 4. 윈도우 소켓 종료 =

		WSACleanup();
		-> 윈속 사용 중단을 운영체제에 알리고 라이브러리를 해제합니다.

		※ 참고
			이 함수가 실패하면 WSAGetLastError로 내막을 알아낼 수 있습니다.
			반환값은 0 혹은 실패할 시 SOCKET_ERROR입니다.
	*/
	WSACleanup();

	return 0;
}
