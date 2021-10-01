#ifndef PCH_H
#define PCH_H
#include "stdafx.h"

class GameSprite;
class GameFramework;

class GameScene;
class sceneTitle;
class sceneMainMenu;
class sceneGame;
class sceneScoring;
class sceneSetting;

class GameInstance;
class oSolid;

#define CLIENT_W 800
#define CLIENT_H 600
#define VIEW_W 320
#define VIEW_H 240
#define PORT_W 640
#define PORT_H 480

namespace Render {
void refresh(HWND hwnd);

void transform_set(HDC, XFORM&);
void transform_set_identity(HDC);
void transform_set_rotation(HDC);

void draw_clear(HDC, int, int, COLORREF = 0);
BOOL draw_rectangle(HDC, int, int, int, int);
void draw_end(HDC, HGDIOBJ, HGDIOBJ);

static XFORM transform_identity{ 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };
}

template<typename T>
constexpr auto sign(T _x) { return ((_x > 0) ? 1 : ((_x == 0) ? 0 : -1)); }

inline double point_distance(double x1, double y1, double x2, double y2) {
	return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}

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
#endif
