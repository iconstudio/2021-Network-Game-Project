// Findme.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "stdafx.h"
#include "Findme.h"

#pragma comment(lib, "shcore.lib")
#include <shellscalingapi.h>

#define MAX_LOADSTRING 100
#define RENDER_TIMER_ID 1

WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
WindowsClient game_client{ CLIENT_W, CLIENT_H };
GameFramework framework{ GAME_SCENE_W, GAME_SCENE_H, VIEW_W, VIEW_H, PORT_W, PORT_H };

auto sHumanStand = make_sprite(TEXT("Resource\\player_stand.png"), 1, 8, 8);

const int game_map_iwidth = GAME_SCENE_W / 16;
const int game_map_iheight = GAME_SCENE_H / 16;
char** game_map;

const double PLAYER_MOVE_VELOCITY = km_per_hr(20);
const int PEOPLE_NUMBER = 2000;

const uniform_int_distribution<int> w_distribution{ 16, GAME_SCENE_W - 16 };
const uniform_int_distribution<int> h_distribution{ 16, GAME_SCENE_H - 16 };
default_random_engine game_random_engine;

oPlayer::oPlayer(GameMesh***& mesh) : GameInstance(mesh) {}

void oPlayer::on_create() {
	set_sprite(sHumanStand);
}

void oPlayer::on_update(double frame_advance) {
	int check_left = framework.input_check(VK_LEFT);
	int check_right = framework.input_check(VK_RIGHT);
	int check_up = framework.input_check(VK_UP);
	int check_down = framework.input_check(VK_DOWN);

	int input_hor = check_right - check_left;
	int input_ver = check_down - check_up;
	double move_dir = point_direction(0, 0, input_hor, input_ver);
	if (input_hor != 0 || input_ver != 0) {
		x += lengthdir_x(PLAYER_MOVE_VELOCITY * frame_advance, move_dir);
		y += lengthdir_y(PLAYER_MOVE_VELOCITY * frame_advance, move_dir);
	}

	if (0 != input_hor) {
		//x += input_hor * PLAYER_MOVE_VELOCITY;
		xscale = input_hor;
	}

	GameInstance::on_update(frame_advance);
}

void oPlayer::on_render(HDC canvas) {
	GameInstance::on_render(canvas);
}

oFakePerson::oFakePerson(GameMesh***& mesh) : GameInstance(mesh) {}

void oFakePerson::on_create() {
	set_sprite(sHumanStand);
}

void oFakePerson::on_update(double frame_advance) {
	GameInstance::on_update(frame_advance);
}

void oFakePerson::on_render(HDC canvas) {
	GameInstance::on_render(canvas);
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	::SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_FINDME, szWindowClass, MAX_LOADSTRING);

	if (!game_client.initialize(hInstance, WndProc, szTitle, szWindowClass, nCmdShow)) {
		return FALSE;
	}

	framework.init();
	framework.set_view_tracking(true);
	framework.input_register(VK_LEFT);
	framework.input_register(VK_RIGHT);
	framework.input_register(VK_UP);
	framework.input_register(VK_DOWN);

	game_map = new char*[game_map_iheight];
	for (int i = 0; i < game_map_iheight; ++i) {
		game_map[i] = new char[game_map_iwidth];
		ZeroMemory(game_map[i], game_map_iwidth);
	}

	framework.background_color = COLOR_GOLD;

	for (int i = 0; i < PEOPLE_NUMBER; ++i) {
		int cx = w_distribution(game_random_engine);
		int cy = h_distribution(game_random_engine);

		framework.instance_create<oFakePerson>(cx, cy);
	}
	auto player = framework.instance_create<oPlayer>(90, 90);
	framework.set_view_target(player);

	MSG msg;
	while (true) {
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		// 창 생성
		case WM_CREATE:
		{
			SetTimer(hwnd, RENDER_TIMER_ID, 1, NULL);
		}
		break;

		// 렌더링 타이머
		case WM_TIMER:
		{
			framework.update();
			Render::refresh(hwnd);
		}
		break;

		// 마우스 왼쪽 누름
		case WM_LBUTTONDOWN:
		{
			framework.on_mousedown(MK_LBUTTON, lParam);
		}
		break;

		// 마우스 왼쪽 뗌
		case WM_LBUTTONUP:
		{
			framework.on_mouseup(MK_LBUTTON, lParam);
		}
		break;

		// 마우스 오른쪽 누름
		case WM_RBUTTONDOWN:
		{
			framework.on_mousedown(MK_RBUTTON, lParam);
		}
		break;

		// 마우스 오른쪽 뗌
		case WM_RBUTTONUP:
		{
			framework.on_mouseup(MK_RBUTTON, lParam);
		}
		break;

		// 마우스 휠 누름
		case WM_MBUTTONDOWN:
		{
			framework.on_mousedown(MK_MBUTTON, lParam);
		}
		break;

		// 마우스 휠 뗌
		case WM_MBUTTONUP:
		{
			framework.on_mouseup(MK_MBUTTON, lParam);
		}
		break;

		// 렌더링
		case WM_PAINT:
		{
			framework.draw(hwnd);
		}
		break;

		// 창 종료
		case WM_DESTROY:
		{
			KillTimer(hwnd, RENDER_TIMER_ID);
			PostQuitMessage(0);
		}
		break;

		default:
			return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}
