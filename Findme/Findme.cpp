// Findme.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//
#include "stdafx.h"
#include "Findme.h"

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

const int enemy_number = 2000;

const uniform_int_distribution<int> game_distribution;
const default_random_engine game_random_engine;
const random_device game_random_device;

oPlayer::oPlayer(char** mesh) : GameInstance(mesh) {}

void oPlayer::on_create() {
	set_sprite(sHumanStand);
}

void oPlayer::on_update(double frame_advance) {
	GameInstance::on_update(frame_advance);
}

void oPlayer::on_render(HDC canvas) {
	GameInstance::on_render(canvas);
}

oFakePerson::oFakePerson(char** mesh) {}

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

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_FINDME, szWindowClass, MAX_LOADSTRING);

	if (!game_client.initialize(hInstance, WndProc, szTitle, szWindowClass, nCmdShow)) {
		return FALSE;
	}

	framework.init();

	game_map = new char*[game_map_iheight];
	for (int i = 0; i < game_map_iheight; ++i) {
		game_map[i] = new char[game_map_iwidth];
		ZeroMemory(game_map[i], game_map_iwidth);
	}

	framework.background_color = COLOR_GOLD;

	auto player = framework.instance_create<oPlayer>(90, 90);
	for (int i = 0; i < enemy_number; ++i) {
		int cx = game_distribution(game_random_engine);

	}

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
			Render::refresh(hwnd);
			framework.update();
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
