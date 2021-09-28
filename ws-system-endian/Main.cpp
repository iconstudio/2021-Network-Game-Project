#pragma comment(lib, "ws2_32")

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <stdio.h>
#include <WinSock2.h>
#include <array>


/// �ü������ ��Ʋ ������� ����ϸ� TRUE, �׷��� ������ FALSE�� ��ȯ�մϴ�.
BOOL IsLittleEndian() {
	puts("[�ý��� ����Ʈ �Ǻ�: IsLittleEndian]\n");

	/*
		�� ��ü�� �ƹ� ���ĵ� �ƴϱ⿡ ������ ���� ��� �ִ� �����͸� ���� �о��� ��
		����Ʈ ���� �ɰ��� ���� ������ ���� ����� �մϴ�.

		0x85916632: ������ �ֻ��� ����Ʈ, �������� ������ ����Ʈ

			85	00	00	00
			00	91	00	00
			00	00	66	00
			00	00	00	32
	*/
	const u_long value_test = 0x85916632;

	/*
		����Ʈ ������ Ž���ϱ� ���� unsigned long�� ũ��(����Ʈ ����)�� ���մϴ�.
	*/
	const auto times = sizeof(value_test); // 4����Ʈ

	/*
		value_test�� �ּ�
		char*�� ũ��� 1����Ʈ��, char* �����Ϳ� 1�� ���ϸ� ����Ű�� ��ġ�� 1����Ʈ �����մϴ�.
	*/
	char* value_ptr = (char*)(&value_test);

	puts("[ȣ��Ʈ ����Ʈ ����]");
	printf("���� ��: %#x\n", value_test);
	printf("�ּ� ������ ���: ");

	// 1�� ���ϸ� �޸𸮿� ����� ����Ʈ�� ������� �н��ϴ�.
	for (int i = 0; i < times; i++) {
		u_long value_now = value_ptr[i];

		printf("%#x ", value_now);
	}
	puts("\n");
	/*
		- ��� ���: 0x32 0x66 0xffffff91 0xffffff85

		value_test�� ������ ����Ʈ�� 0x32�� ���� �������� �� ù��°�� ��ġ�� �ֽ��ϴ�.
	*/

	/*
		- ���:

		�޸𸮸� ���� ���� ��� value_test�� ������ ����Ʈ���� �������Ƿ� �ü���� ��Ʋ ����� ������ ����մϴ�.

		�ڵ�δ� 'ù��° �ּҿ� �ִ� ��'�� '������ ����Ʈ�� �ִ� ��'�� ���ؼ� ��Ʋ ������� �Ǻ��� �� �ֽ��ϴ�.

		�׷��� ���� �յڰ� �Ȱ��� ���̶�� �Ǻ��� �Ұ����մϴ�.

		�׷��� �ּҴ� ó������, ����Ʈ�� �������� �̾� ���� ���մϴ�.
	*/

	constexpr u_long byte_high = HIWORD(value_test);		// ���� 2����Ʈ (0x8591)
	constexpr u_long byte_high_high = HIBYTE(byte_high);	// ���� 1����Ʈ (0x85)
	constexpr u_long byte_high_low = LOBYTE(byte_high); 	// ���� 1����Ʈ (0x91)

	constexpr u_long byte_low = LOWORD(value_test); 		// ���� 2����Ʈ (0x6632)
	constexpr u_long byte_low_high = HIBYTE(byte_low); 		// ���� 1����Ʈ (0x66)
	constexpr u_long byte_low_low = LOBYTE(byte_low); 		// ���� 1����Ʈ (0x32)

	BYTE on_the_memory_first = value_ptr[0];				// ù��° �ּ��� ��
	BYTE on_the_memory_second = value_ptr[1];				// �ι�° �ּ��� ��
	BYTE on_the_memory_third = value_ptr[2];				// ����° �ּ��� ��
	BYTE on_the_memory_fourth = value_ptr[3];				// �׹�° �ּ��� ��

	printf("�ֻ��� ����Ʈ ��: %#x\n", byte_high_high);
	printf("�ι�° ����Ʈ ��: %#x\n", byte_high_low);
	printf("����° ����Ʈ ��: %#x\n", byte_low_high);
	printf("������ ����Ʈ �� : % #x\n\n", byte_low_low);

	printf("ù��° �ּ� ��: %#x\n", on_the_memory_first);
	printf("�ι�° �ּ� ��: %#x\n", on_the_memory_second);
	printf("����° �ּ� ��: %#x\n", on_the_memory_third);
	printf("�׹�° �ּ� ��: %#x\n\n", on_the_memory_fourth);
	puts("\n");

	if (byte_low_low == on_the_memory_first
		&& byte_low_high == on_the_memory_second
		&& byte_high_low == on_the_memory_third
		&& byte_high_high == on_the_memory_fourth)
		return TRUE;

	return FALSE;
}

/// �ü������ �� ������� ����ϸ� TRUE, �׷��� ������ FALSE�� ��ȯ�մϴ�.
BOOL IsBigEndian() {
	puts("[�ý��� ����Ʈ �Ǻ�: IsBigEndian]\n");

	const u_long value_test = 0x491708; // 0x00491708

	/*
		����Ʈ ������ Ž���ϱ� ���� unsigned long�� ũ��(����Ʈ ����)�� ���մϴ�.
	*/
	const auto times = sizeof(value_test);

	/*
		value_test�� �ּ�
		char*�� ũ��� 1����Ʈ��, char* �����Ϳ� 1�� ���ϸ� ����Ű�� ��ġ�� 1����Ʈ �����մϴ�.
	*/
	char* value_ptr = (char*)(&value_test);

	puts("[ȣ��Ʈ ����Ʈ ����]");
	printf("���� ��: %#x\n", value_test);
	printf("�ּ� ������ ���: ");

	// 1�� ���ϸ� �޸𸮿� ����� ����Ʈ�� ������� �н��ϴ�.
	for (int i = 0; i < times; i++)
		printf("%#x ", value_ptr[i]);
	puts("\n");

	/*
		- ��� ���: 0x8 0x17 0x49 0x0

		value_test�� ������ ����Ʈ�� ���� �������� �� ù��°�� ��ġ�� �ֽ��ϴ�.
	*/

	constexpr u_long aaa = 0x1708;
	constexpr u_long byte_high = HIWORD(value_test);		// ���� 2����Ʈ (0x0049)
	constexpr u_long byte_high_high = HIBYTE(byte_high);	// ���� 1����Ʈ (0x00)
	constexpr u_long byte_high_low = LOBYTE(byte_high); 	// ���� 1����Ʈ (0x49)

	constexpr u_long byte_low = LOWORD(value_test); 		// ���� 2����Ʈ (0x1708)
	constexpr u_long byte_low_high = HIBYTE(byte_low); 		// ���� 1����Ʈ (0x17)
	constexpr u_long byte_low_low = LOBYTE(byte_low); 		// ���� 1����Ʈ (0x08)

	BYTE on_the_memory_first = value_ptr[0];				// ù��° �ּ��� ��
	BYTE on_the_memory_second = value_ptr[1];				// �ι�° �ּ��� ��
	BYTE on_the_memory_third = value_ptr[2];				// ����° �ּ��� ��
	BYTE on_the_memory_fourth = value_ptr[3];				// �׹�° �ּ��� ��

	printf("�ֻ��� ����Ʈ ��: %#x\n", byte_high_high);
	printf("�ι�° ����Ʈ ��: %#x\n", byte_high_low);
	printf("����° ����Ʈ ��: %#x\n", byte_low_high);
	printf("������ ����Ʈ �� : % #x\n\n", byte_low_low);

	printf("ù��° �ּ� ��: %#x\n", on_the_memory_first);
	printf("�ι�° �ּ� ��: %#x\n", on_the_memory_second);
	printf("����° �ּ� ��: %#x\n", on_the_memory_third);
	printf("�׹�° �ּ� ��: %#x\n\n", on_the_memory_fourth);
	puts("\n");

	if (byte_low_low == on_the_memory_fourth
		&& byte_low_high == on_the_memory_third
		&& byte_high_low == on_the_memory_second
		&& byte_high_high == on_the_memory_first)
		return TRUE;

	return FALSE;
}

/*
	�Ʒ��� �� �Լ��� ���� �ʱ⿡ �������� �ϴٰ� �ϵ� �ڵ����� ���� �Լ����Դϴ�.
	IsLittleEndianBitshift�� IsLittleEndian�� HIWORD, HIBYTE...�� �𸣰� ®�� �����Դϴ�.
	IsBigEndianSimple�� ���� �˻��� ���� �˾Ƴ´� �����Դϴ�.
*/

/// �ü������ ��Ʋ ������� ����ϸ� TRUE, �׷��� ������ FALSE�� ��ȯ�մϴ�.
/// ��Ʈ ������ ����߽��ϴ�.
BOOL IsLittleEndianBitshift() {
	const u_long value_test = 0x85916632;
	const auto times = sizeof(value_test); // 4����Ʈ
	char* value_ptr = (char*)(&value_test);

	BYTE on_the_memory; // �ּҿ� �ִ� ��, �� ���� �׻� 0xff �����Դϴ�.
	u_long byte_value; // ����Ʈ ��

	const u_long memory_mask = 0x000000ff; // ���� �߶󳻱� ����ũ
	const u_int byte_size = 8; // 1����Ʈ = 8��Ʈ
	u_long bit_mask = 0x000000ff; // ����Ʈ �߶󳻱� ����ũ
	u_int bit_pusher = 0;

	/*
		on_the_memory������ i ��ġ�� �ּҿ� �ִ� ���� �о�ͼ� 0xff�� ����ŷ�� �մϴ�.
		value_ptr[i]�� 32��Ʈ �����ε�, ù��° 1����Ʈ�� �߶󳻸� �ش� ���� ���ɴϴ�.

		- value_ptr[i]�� �� ó�� ���� ��Ʋ ����ȿ����� value_test�� ������ ����Ʈ�Դϴ�.

		byte_value������ bit_pusher Ƚ����ŭ ����Ʈ�� �о ���� �н��ϴ�.
		bit_mask�� value_test�� ��Ʈ and ������ �ϸ� �ش� ��Ʈ���� ���� 0�� �˴ϴ�.
		�׸��� bit_pusher�� ��� ��Ʈ�� 1����Ʈ �� ���������� �ű�ϴ�.
		��Ȯ�� ������ �д� ��ġ �������� ������ ��Ʈ ����Ʈ�� ���ִ� ���Դϴ�.
		�̷��� �о�� �� ��Ʈ���� ������ ��ġ�� �̵��� �Ǿ �����ϰ� ���� ���� �� �ֽ��ϴ�.

		- byte_value�� �� ó�� ���� value_test�� �� ù��° ����Ʈ�Դϴ�.

		on_the_memory�� i�� ������ ���� ��ġ�� ã����, byte_value�� bit_mask�� ���� ���������� �����ϴ�.
	*/
	for (int i = 0; i < times; i++) {
		on_the_memory = memory_mask & (value_ptr[i]);

		byte_value = (bit_mask & value_test);
		if (0 < bit_pusher)
			byte_value >>= (bit_pusher * byte_size);

		bit_pusher++; // 1, 2, 3, 4
		bit_mask <<= byte_size; // 1 ����Ʈ �� �Ӵϴ�.

		if (on_the_memory != byte_value)
			return FALSE;
	}
	return TRUE;
}

int main(void) {
	WSADATA wsa; // ������ ���� ����

	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
		return 1;
	}

	auto is_little = IsLittleEndian();
	auto is_big = IsBigEndian();

	const std::array<const char*, 2> BOOLEAN_STR = { "FALSE", "TRUE" };
	printf("�ý����� ��Ʋ ����� ����Ʈ ������ ����ϴ°�?: %s\n", BOOLEAN_STR[is_little]);
	printf("�ý����� �� ����� ����Ʈ ������ ����ϴ°�?: %s\n", BOOLEAN_STR[is_big]);
	puts("\n");

	WSACleanup();

	return 0;
}
