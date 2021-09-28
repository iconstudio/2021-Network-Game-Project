#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <WinSock2.h>
#include <array>


/// 운영체제에서 리틀 엔디안을 사용하면 TRUE, 그렇지 않으면 FALSE를 반환합니다.
BOOL IsLittleEndian() {
	puts("[시스템 바이트 판별: IsLittleEndian]\n");

	/*
		값 자체는 아무 정렬도 아니기에 변수의 값을 들고 있는 포인터를 따라 읽었을 때
		바이트 별로 쪼개진 값의 순서를 보고 맞춰야 합니다.

		0x85916632: 왼쪽이 최상위 바이트, 오른쪽이 최하위 바이트

			85	00	00	00
			00	91	00	00
			00	00	66	00
			00	00	00	32
	*/
	const u_long value_test = 0x85916632;

	/*
		바이트 단위로 탐사하기 위해 unsigned long의 크기(바이트 갯수)를 구합니다.
	*/
	const auto times = sizeof(value_test); // 4바이트

	/*
		value_test의 주소
		char*의 크기는 1바이트라서, char* 포인터에 1을 더하면 가리키는 위치가 1바이트 증가합니다.
	*/
	char* value_ptr = (char*)(&value_test);

	puts("[호스트 바이트 정렬]");
	printf("원래 값: %#x\n", value_test);
	printf("주소 순으로 출력: ");

	// 1씩 더하며 메모리에 저장된 바이트를 순서대로 읽습니다.
	for (int i = 0; i < times; i++) {
		u_long value_now = value_ptr[i];

		printf("%#x ", value_now);
	}
	puts("\n");
	/*
		- 출력 결과: 0x32 0x66 0xffffff91 0xffffff85

		value_test의 최하위 바이트인 0x32가 저장 순서에선 맨 첫번째에 위치해 있습니다.
	*/

	/*
		- 결론:

		메모리를 직접 읽은 결과 value_test의 최하위 바이트부터 읽혔으므로 운영체제는 리틀 엔디안 정렬을 사용합니다.

		코드로는 '첫번째 주소에 있는 값'과 '최하위 바이트에 있는 값'을 비교해서 리틀 엔디안을 판별할 수 있습니다.

		그런데 만약 앞뒤가 똑같은 값이라면 판별이 불가능합니다.

		그래서 주소는 처음부터, 바이트는 하위부터 뽑아 서로 비교합니다.
	*/

	constexpr u_long byte_high = HIWORD(value_test);		// 상위 2바이트 (0x8591)
	constexpr u_long byte_high_high = HIBYTE(byte_high);	// 상위 1바이트 (0x85)
	constexpr u_long byte_high_low = LOBYTE(byte_high); 	// 상위 1바이트 (0x91)

	constexpr u_long byte_low = LOWORD(value_test); 		// 하위 2바이트 (0x6632)
	constexpr u_long byte_low_high = HIBYTE(byte_low); 		// 하위 1바이트 (0x66)
	constexpr u_long byte_low_low = LOBYTE(byte_low); 		// 하위 1바이트 (0x32)

	BYTE on_the_memory_first = value_ptr[0];				// 첫번째 주소의 값
	BYTE on_the_memory_second = value_ptr[1];				// 두번째 주소의 값
	BYTE on_the_memory_third = value_ptr[2];				// 세번째 주소의 값
	BYTE on_the_memory_fourth = value_ptr[3];				// 네번째 주소의 값

	printf("최상위 바이트 값: %#x\n", byte_high_high);
	printf("두번째 바이트 값: %#x\n", byte_high_low);
	printf("세번째 바이트 값: %#x\n", byte_low_high);
	printf("최하위 바이트 값 : % #x\n\n", byte_low_low);

	printf("첫번째 주소 값: %#x\n", on_the_memory_first);
	printf("두번째 주소 값: %#x\n", on_the_memory_second);
	printf("세번째 주소 값: %#x\n", on_the_memory_third);
	printf("네번째 주소 값: %#x\n\n", on_the_memory_fourth);
	puts("\n");

	if (byte_low_low == on_the_memory_first
		&& byte_low_high == on_the_memory_second
		&& byte_high_low == on_the_memory_third
		&& byte_high_high == on_the_memory_fourth)
		return TRUE;

	return FALSE;
}

/// 운영체제에서 빅 엔디안을 사용하면 TRUE, 그렇지 않으면 FALSE를 반환합니다.
BOOL IsBigEndian() {
	puts("[시스템 바이트 판별: IsBigEndian]\n");

	const u_long value_test = 0x491708; // 0x00491708

	/*
		바이트 단위로 탐사하기 위해 unsigned long의 크기(바이트 갯수)를 구합니다.
	*/
	const auto times = sizeof(value_test);

	/*
		value_test의 주소
		char*의 크기는 1바이트라서, char* 포인터에 1을 더하면 가리키는 위치가 1바이트 증가합니다.
	*/
	char* value_ptr = (char*)(&value_test);

	puts("[호스트 바이트 정렬]");
	printf("원래 값: %#x\n", value_test);
	printf("주소 순으로 출력: ");

	// 1씩 더하며 메모리에 저장된 바이트를 순서대로 읽습니다.
	for (int i = 0; i < times; i++)
		printf("%#x ", value_ptr[i]);
	puts("\n");

	/*
		- 출력 결과: 0x8 0x17 0x49 0x0

		value_test의 최하위 바이트가 저장 순서에선 맨 첫번째에 위치해 있습니다.
	*/

	constexpr u_long aaa = 0x1708;
	constexpr u_long byte_high = HIWORD(value_test);		// 상위 2바이트 (0x0049)
	constexpr u_long byte_high_high = HIBYTE(byte_high);	// 상위 1바이트 (0x00)
	constexpr u_long byte_high_low = LOBYTE(byte_high); 	// 상위 1바이트 (0x49)

	constexpr u_long byte_low = LOWORD(value_test); 		// 하위 2바이트 (0x1708)
	constexpr u_long byte_low_high = HIBYTE(byte_low); 		// 하위 1바이트 (0x17)
	constexpr u_long byte_low_low = LOBYTE(byte_low); 		// 하위 1바이트 (0x08)

	BYTE on_the_memory_first = value_ptr[0];				// 첫번째 주소의 값
	BYTE on_the_memory_second = value_ptr[1];				// 두번째 주소의 값
	BYTE on_the_memory_third = value_ptr[2];				// 세번째 주소의 값
	BYTE on_the_memory_fourth = value_ptr[3];				// 네번째 주소의 값

	printf("최상위 바이트 값: %#x\n", byte_high_high);
	printf("두번째 바이트 값: %#x\n", byte_high_low);
	printf("세번째 바이트 값: %#x\n", byte_low_high);
	printf("최하위 바이트 값 : % #x\n\n", byte_low_low);

	printf("첫번째 주소 값: %#x\n", on_the_memory_first);
	printf("두번째 주소 값: %#x\n", on_the_memory_second);
	printf("세번째 주소 값: %#x\n", on_the_memory_third);
	printf("네번째 주소 값: %#x\n\n", on_the_memory_fourth);
	puts("\n");

	if (byte_low_low == on_the_memory_fourth
		&& byte_low_high == on_the_memory_third
		&& byte_high_low == on_the_memory_second
		&& byte_high_high == on_the_memory_first)
		return TRUE;

	return FALSE;
}

/*
	아래의 두 함수는 과제 초기에 갈팡질팡 하다가 하드 코딩으로 만든 함수들입니다.
	IsLittleEndianBitshift은 IsLittleEndian을 HIWORD, HIBYTE...를 모르고 짰던 내용입니다.
	IsBigEndianSimple은 구글 검색을 통해 알아냈던 내용입니다.
*/

/// 운영체제에서 리틀 엔디안을 사용하면 TRUE, 그렇지 않으면 FALSE를 반환합니다.
/// 비트 연산을 사용했습니다.
BOOL IsLittleEndianBitshift() {
	const u_long value_test = 0x85916632;
	const auto times = sizeof(value_test); // 4바이트
	char* value_ptr = (char*)(&value_test);

	BYTE on_the_memory; // 주소에 있는 값, 이 값은 항상 0xff 이하입니다.
	u_long byte_value; // 바이트 값

	const u_long memory_mask = 0x000000ff; // 워드 잘라내기 마스크
	const u_int byte_size = 8; // 1바이트 = 8비트
	u_long bit_mask = 0x000000ff; // 바이트 잘라내기 마스크
	u_int bit_pusher = 0;

	/*
		on_the_memory에서는 i 위치의 주소에 있는 값을 읽어와서 0xff로 마스킹을 합니다.
		value_ptr[i]는 32비트 워드인데, 첫번째 1바이트만 잘라내면 해당 값이 나옵니다.

		- value_ptr[i]의 맨 처음 값은 리틀 엔디안에서는 value_test의 최하위 바이트입니다.

		byte_value에서는 bit_pusher 횟수만큼 바이트를 밀어서 값을 읽습니다.
		bit_mask와 value_test로 비트 and 연산을 하면 해당 비트빼고 전부 0이 됩니다.
		그리고 bit_pusher로 모든 비트를 1바이트 씩 오른쪽으로 옮깁니다.
		정확한 원리는 읽는 위치 오른쪽의 값들을 비트 쉬프트로 없애는 것입니다.
		이러면 읽어야 할 비트들이 최하위 위치로 이동이 되어서 수월하게 값을 읽을 수 있습니다.

		- byte_value의 맨 처음 값은 value_test의 맨 첫번째 바이트입니다.

		on_the_memory는 i로 스스로 읽을 위치를 찾지만, byte_value는 bit_mask에 의해 수동적으로 읽힙니다.
	*/
	for (int i = 0; i < times; i++) {
		on_the_memory = memory_mask & (value_ptr[i]);

		byte_value = (bit_mask & value_test);
		if (0 < bit_pusher)
			byte_value >>= (bit_pusher * byte_size);

		bit_pusher++; // 1, 2, 3, 4
		bit_mask <<= byte_size; // 1 바이트 씩 밉니다.

		if (on_the_memory != byte_value)
			return FALSE;
	}
	return TRUE;
}

int main(void) {
	WSADATA wsa; // 윈도우 소켓 정보

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	auto is_little = IsLittleEndian();
	auto is_big = IsBigEndian();

	const std::array<const char*, 2> BOOLEAN_STR = { "FALSE", "TRUE" };
	printf("시스템이 리틀 엔디안 바이트 정렬을 사용하는가?: %s\n", BOOLEAN_STR[is_little]);
	printf("시스템이 빅 엔디안 바이트 정렬을 사용하는가?: %s\n", BOOLEAN_STR[is_big]);
	puts("\n");

	WSACleanup();

	return 0;
}
