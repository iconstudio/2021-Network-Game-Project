#pragma comment(lib, "ws2_32")
#include <WinSock2.h> // 윈도우 소켓 2.2 API 불러오기
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
	WSADATA wsa; // 윈도우 소켓 정보

	const WORD version_request = MAKEWORD(2, 0); // (주 버전, 부 버전)
	if (WSAStartup(version_request, &wsa) != 0) {
		return 1;
	}

	printf("초기화 할 버전: %d.%d\n", LOBYTE(version_request), HIBYTE(version_request));
	BYTE highversion_sub = HIBYTE(wsa.wHighVersion);
	BYTE highversion_main = LOBYTE(wsa.wHighVersion);
	BYTE version_sub = HIBYTE(wsa.wVersion);
	BYTE version_main = LOBYTE(wsa.wVersion);

	printf("윈속 라이브러리에서 지원하는 가장 높은 버전(wHighVersion): %d.%d\n", highversion_main, highversion_sub);
	printf("현재 실행 중인 윈속 버전 (wVersion): %d.%d\n", version_main, version_sub);
	printf("윈속 라이브러리 설명 (szDescription): %s\n", wsa.szDescription);
	printf("윈속 상태 또는 라이브러리 구성 정보 (szSystemStatus): %s \n", wsa.szSystemStatus);

	MessageBox(NULL, "윈속 초기화 성공", "알림", MB_OK);

	WSACleanup();

	return 0;
}
