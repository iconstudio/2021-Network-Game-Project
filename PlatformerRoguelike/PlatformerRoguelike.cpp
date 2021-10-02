#include "pch.h"
#include "PlatformerRoguelike.h"
#include "Framework.h"

#define MAX_LOADSTRING 100
#define RENDER_TIMER_ID 1

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
WindowsClient game_client{ CLIENT_W, CLIENT_H };
GameFramework framework{ VIEW_W, VIEW_H, PORT_W, PORT_H };

auto sBall = framework.make_sprite(TEXT("Resources\\ball.png"), 1, 32, 32);
auto sBlock = framework.make_sprite(TEXT("Resources\\blockpurple.png"), 1, 0, 0);
auto sPlayer = framework.make_sprite(TEXT("Resources\\player.png"), 1, 6, 6);


LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PLATFORMERROGUELIKE, szWindowClass, MAX_LOADSTRING);
	
	if (!game_client.initialize(hInstance, WndProc, szTitle, szWindowClass, nCmdShow)) {
		return FALSE;
	}

	framework.make_scene<sceneGame>();

	framework.init();

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

void oGraviton::on_update(double frame_advance) {}

double oGraviton::raycast(double distance, StaticDir direction) {
	return 0.0;
}

GameWorld::GameWorld() {}

GameWorld::~GameWorld() {}

void sceneGame::on_create() {
	map_blend.SourceConstantAlpha = 255;
	map_blend.BlendOp = AC_SRC_OVER;
	map_blend.BlendFlags = 0;
	map_blend.AlphaFormat = AC_SRC_ALPHA;
	//framework.background_color = 0;
	framework.background_color = COLOR_GREY;

	build_terrain = new char*[TILE_JMAX];
	for (int j = 0; j < TILE_JMAX; ++j) {
		build_terrain[j] = new char[TILE_IMAX];
		ZeroMemory(build_terrain[j], TILE_IMAX);
	}

	FILE* myfile = nullptr;
	auto result = fopen_s(&myfile, "map.txt", "r+");
	if (myfile) {
		int j = 0;
		while (!feof(myfile)) {
			fread(build_terrain[j], sizeof(char), TILE_IMAX, myfile);
			fgetc(myfile); // throw a \n.
			if (TILE_JMAX <= ++j) break;
		}

		fclose(myfile);
	} else {
		if (MessageBox(NULL, TEXT("게임 맵 파일을 찾을 수 없습니다."), TEXT("오류"), MB_OK | MB_ICONERROR)) {
			DestroyWindow(game_client.hwindow);
			return;
		}
	}

	HDC world_dc = GetDC(NULL);
	map_surface = CreateCompatibleDC(world_dc);
	map_bitmap = CreateCompatibleBitmap(world_dc, VIEW_W, VIEW_H);
	SelectObject(map_surface, map_bitmap);

	Render::draw_clear(map_surface, VIEW_W, VIEW_H, 0);

	auto m_hPen = CreatePen(PS_NULL, 1, COLOR_GREEN);
	auto m_oldhPen = (HPEN)SelectObject(map_surface, m_hPen);
	auto m_hBR = CreateSolidBrush(COLOR_GREEN);
	auto m_oldhBR = (HBRUSH)SelectObject(map_surface, m_hBR);
	int i, j;
	double cx, cy;
	char block_data;
	auto pp = instance_create<GameInstance>(58.0, 208.0);
	pp->set_sprite(sPlayer);

	for (j = 0; j < TILE_JMAX; ++j) {
		for (i = 0; i < TILE_IMAX; ++i) {
			block_data = (build_terrain[j])[i];

			if (block_data != '0') {
				cx = i * 16.0;
				cy = j * 16.0;
				switch (block_data) {
					case '@':
					{
						pp = instance_create<oPlayer>(cx + 8.0, cy + 8.0);
					}
					break;

					case '1':
					{
						// 블록
						//Rectangle(map_surface, i * 16, j * 16, (i + 1) * 16, (j + 1) * 16);
						//auto block = instance_create<GameInstance>(i * 16.0, j * 16.0);
						//block->set_sprite(sBlock);
						sBlock->draw(map_surface, cx, cy);
					}
					break;

					default:
						break;
				}
			}
		}
	}
	Render::draw_end(map_surface, m_oldhBR, m_hBR);

	Render::draw_end(map_surface, m_oldhPen, m_hPen);

	//instance_kill(pp.get());

	GameScene::on_create();
}

void sceneGame::on_destroy() {
	DeleteDC(map_surface);
	DeleteObject(map_bitmap);
	GameScene::on_destroy();
}

void sceneGame::on_update(double frame_advance) {
	GameScene::on_update(frame_advance);
}

void sceneGame::on_render(HDC canvas) {
	AlphaBlend(canvas, 0, 0, VIEW_W, VIEW_H, map_surface, 0, 0, VIEW_W, VIEW_H, map_blend);
	GameScene::on_render(canvas);
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
			framework.on_render(hwnd);
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
