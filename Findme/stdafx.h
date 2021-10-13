// header.h: 표준 시스템 포함 파일
// 또는 프로젝트 특정 포함 파일이 들어 있는 포함 파일입니다.
//

#pragma once

#pragma once
#pragma comment(lib, "msimg32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma warning(disable: 4244)

#include "targetver.h"

/// Windows 헤더 파일:
#define WIN32_LEAN_AND_MEAN // 거의 사용되지 않는 내용은 Windows 헤더에서 제외합니다.
#include <windows.h>
#include <winperf.h>

/// C 런타임 헤더 파일입니다.
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <time.h>

/// ATL / MFC 헤더 파일:
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // 일부 CString 생성자는 명시적으로 선언됩니다.
#include <atlbase.h>
#include <atlimage.h>
#include <atlstr.h>

// 수학 상수 선언
#define _USE_MATH_DEFINES
#include <math.h>

// 표준 라이브러리
#include <memory>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <fstream>
#include <cmath>
#include <chrono>
#include <random>

using namespace std;

#define GAME_SCENE_W 320 * 3
#define GAME_SCENE_H 240 * 3
#define CLIENT_W 960
#define CLIENT_H 540
#define VIEW_W 320
#define VIEW_H 240
#define PORT_W 640
#define PORT_H 480

constexpr double meter_to_pixel = 16.;
constexpr double hour_to_seconds = 3600.;

constexpr double kph_to_pps = (1000.0 * meter_to_pixel / hour_to_seconds);

constexpr double km_per_hr(const double velocity) {
	return velocity * kph_to_pps;
}

constexpr COLORREF COLOR_BLACK = RGB(0, 0, 0);
constexpr COLORREF COLOR_WHITE = RGB(255, 255, 255);
constexpr COLORREF COLOR_GREY = RGB(128, 128, 128);
constexpr COLORREF COLOR_LTGREY = RGB(192, 192, 192);
constexpr COLORREF COLOR_DKGREY = RGB(64, 64, 64);
constexpr COLORREF COLOR_SEMIBLACK = RGB(224, 224, 224);

constexpr COLORREF COLOR_SALMON = RGB(255, 128, 128);
constexpr COLORREF COLOR_RED = RGB(255, 0, 0);
constexpr COLORREF COLOR_MAROON = RGB(128, 0, 0);
constexpr COLORREF COLOR_LIME = RGB(0, 255, 0);
constexpr COLORREF COLOR_GREEN = RGB(0, 128, 0);
constexpr COLORREF COLOR_BLUE = RGB(0, 0, 255);
constexpr COLORREF COLOR_NAVY = RGB(20, 18, 149);
constexpr COLORREF COLOR_ORANGE = RGB(255, 128, 0);
constexpr COLORREF COLOR_YELLOW = RGB(255, 255, 0);
constexpr COLORREF COLOR_GOLD = RGB(223, 130, 20);

namespace Render {
void refresh(HWND hwnd);

void transform_set(HDC, XFORM&);
void transform_set_identity(HDC);
void transform_set_rotation(HDC);

void draw_clear(HDC, int width, int height, COLORREF color = 0);
BOOL draw_rectangle(HDC, int, int, int, int);
void draw_end(HDC, HGDIOBJ, HGDIOBJ);

static XFORM transform_identity{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
}

template<typename T>
constexpr auto sign(T _x) { return ((_x > 0) ? 1 : ((_x == 0) ? 0 : -1)); }

inline double radtodeg(double value) {
	return value / M_PI * 180;
}

inline double degtorad(double value) {
	return value * M_PI / 180;
}

inline double lengthdir_x(double length, double direction) {
	return cos(degtorad(direction)) * length;
}

inline double lengthdir_y(double length, double direction) {
	return -sin(degtorad(direction)) * length;
}

inline double point_distance(double x1, double y1, double x2, double y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

inline double point_direction(double x1, double y1, double x2, double y2) {
	return radtodeg(atan2(y1 - y2, x2 - x1));
}

typedef LRESULT(CALLBACK* WindowProcedure)(HWND, UINT, WPARAM, LPARAM);

class WindowsClient {
public:
	WindowsClient(LONG width, LONG height);
	~WindowsClient();

	BOOL initialize(HINSTANCE handle, WNDPROC procedure, LPCWSTR title, LPCWSTR id, INT cmd_show);

	HINSTANCE instance;						// 프로세스 인스턴스
	HWND hwindow;							// 창 인스턴스
	WindowProcedure procedure;				// 창 처리기
	WNDCLASSEX properties;					// 창 등록정보
	LPCWSTR title_caption, class_id;		// 창 식별자
	LONG width, height;						// 창 크기
};
