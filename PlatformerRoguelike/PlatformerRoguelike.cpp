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
auto sPlate = framework.make_sprite(TEXT("Resources\\plate.png"), 1, 0, 0);
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
	framework.input_register(VK_LEFT);
	framework.input_register(VK_RIGHT);
	framework.input_register(VK_UP);
	framework.input_register(VK_DOWN);
	framework.input_register(VK_SPACE);
	framework.input_register(VK_RETURN);
	framework.input_register('R');

	framework.input_register(VK_F1); // 도움말
	framework.input_register(VK_F2); // 게임 전체 다시 시작
	framework.input_register(VK_F3); // 현재 장면만 다시 시작

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

oGraviton::oGraviton(GameWorldMesh* newmesh, double x, double y)
	: GameInstance(x, y), worldmesh(newmesh) {}

void oGraviton::on_create() {
	hspeed = 0.0;
	vspeed = 0.0;
	gravity = GRAVITY;
}

void oGraviton::on_update(double frame_advance) {
	if (0 != hspeed) {
		double cast_x, distance_x = abs(hspeed) * frame_advance;

		if (0 < hspeed) {
			cast_x = raycast_rt(distance_x);
		} else {
			cast_x = raycast_lt(distance_x);
		}

		if (0 < cast_x) {
			if (0 < hspeed) { // right

			} else { // left

			}

			hspeed = 0.0;
		}
	}

	double cast_y;
	double distance_y = abs(vspeed) * frame_advance;
	if (vspeed < 0) { // upward
		cast_y = raycast_up(distance_y);
	} else {
		cast_y = raycast_dw(distance_y);
	}

	if (worldmesh->place_collider(x, bbox_bottom() + 1)) {
		vspeed += gravity * frame_advance;
	} else {
		vspeed = 0.0;
	}
	if (0 < cast_y)
		vspeed = 0.0;
}

void oGraviton::set_worldmesh(GameWorldMesh* newworld) {
	worldmesh = newworld;
}

double oGraviton::raycast_lt(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (!worldmesh->place_free(bbox_left() - 1, bbox_top())
			|| !worldmesh->place_free(bbox_left() - 1, bbox_bottom())) {
			break;
		}
		x--;
		move_distance--;
	}

	return move_distance;
}

double oGraviton::raycast_rt(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (!worldmesh->place_free(bbox_right() + 1, bbox_top())
			|| !worldmesh->place_free(bbox_right() + 1, bbox_bottom())) {
			break;
		}
		x++;
		move_distance--;
	}

	return move_distance;
}

double oGraviton::raycast_up(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (!worldmesh->place_free(bbox_left(), bbox_top() - 1)
			|| !worldmesh->place_free(bbox_right(), bbox_top() - 1)) {
			break;
		}
		y--;
		move_distance--;
	}

	return move_distance;
}

double oGraviton::raycast_dw(double distance) {
	double move_distance = floor(distance * 400) / 400;
	double lx = bbox_left();
	double rx = bbox_right();

	while (0 < move_distance) {
		double by = bbox_bottom() + 1;
		if (!worldmesh->place_free(lx, by)
			|| !worldmesh->place_free(rx, by)) {
			break;
		}

		bool self_in = worldmesh->place_throughable(x, y);
		auto self_check = worldmesh->place_terrain(x, y);
		auto bot_check = worldmesh->place_terrain(x, by);
		auto next_check = worldmesh->place_terrain(x, y + 16);

		if (!self_check) {
			if (worldmesh->place_throughable(lx, by) || worldmesh->place_throughable(rx, by))
				break;
		} else if (self_in) {
			if (next_check && self_check != next_check)
				break;
		}
		y++;
		move_distance--;
	}

	return move_distance;
}

void oGraviton::jump(double power) {
	vspeed = -power;
}

oPlayer::oPlayer(GameWorldMesh* newmesh, double x, double y)
	: oGraviton(newmesh, x, y) {
	set_sprite(sPlayer);
}

void oPlayer::on_create() {
	oGraviton::on_create();
}

void oPlayer::on_update(double frame_advance) {
	int check_left = framework.input_check(VK_LEFT);
	int check_right = framework.input_check(VK_RIGHT);
	int check_up = framework.input_check(VK_UP);
	int check_down = framework.input_check(VK_DOWN);

	bool check_jump = framework.input_check_pressed(VK_SPACE);
	bool check_jump_cnt = framework.input_check(VK_SPACE);

	int check_hor = (check_right - check_left);

	//if (0 != check_hor) {
	hspeed = check_hor * 10.0;
	//}

	if (check_jump) {
		jump(PLAYER_JUMP_VELOCITY);
	}

	oGraviton::on_update(frame_advance);
}

GameWorldMesh::GameWorldMesh(GameScene* room) : my_room(room) {
	map_blend.SourceConstantAlpha = 255;
	map_blend.BlendOp = AC_SRC_OVER;
	map_blend.BlendFlags = 0;
	map_blend.AlphaFormat = AC_SRC_ALPHA;
}

GameWorldMesh::~GameWorldMesh() {
	DeleteDC(map_surface);
	DeleteObject(map_bitmap);
	clear();
}

void GameWorldMesh::load(const char* mapfile) {
	build_terrain.clear();
	build_terrain.reserve(TILE_JMAX * TILE_IMAX + 1);

	FILE* myfile = nullptr;
	auto result = fopen_s(&myfile, mapfile, "r");
	if (myfile) {
		int ch, j = 0;
		while (!feof(myfile)) {
			ch = fgetc(myfile);

			if (ch != '\n') { // \n 버리기.
				build_terrain.push_back(new MeshPiece(ch));
			}
			//fread(build_terrain[j], sizeof(char), TILE_IMAX, myfile);
		}

		fclose(myfile);
	} else {
		if (MessageBox(NULL, TEXT("게임 지도 파일을 찾을 수 없습니다."), TEXT("오류"), MB_OK | MB_ICONERROR)) {
			DestroyWindow(game_client.hwindow);
			return;
		}
	}
}

void GameWorldMesh::build() {
	HDC world_dc = GetDC(NULL);
	map_surface = CreateCompatibleDC(world_dc);
	map_bitmap = CreateCompatibleBitmap(world_dc, VIEW_W, VIEW_H);
	SelectObject(map_surface, map_bitmap);
	Render::draw_clear(map_surface, VIEW_W, VIEW_H, 0);

	auto m_hPen = CreatePen(PS_NULL, 1, COLOR_GREEN);
	auto m_oldhPen = (HPEN)SelectObject(map_surface, m_hPen);
	auto m_hBR = CreateSolidBrush(COLOR_GREEN);
	auto m_oldhBR = (HBRUSH)SelectObject(map_surface, m_hBR);

	int i = 0;
	double cx, cy;

	for (const auto& tile : build_terrain) {
		const char data = tile->data;

		if (data != '0') {
			cx = 16.0 * (i - floor(i / TILE_IMAX) * TILE_IMAX);
			cy = 16.0 * floor(i / TILE_IMAX);

			switch (data) {
				case TILES::PLAYER:
				{
					auto pp = my_room->instance_create<oPlayer>(this, cx + 8.0, cy + 8.0);
				}
				break;

				case TILES::BLOCK:
				{
					sBlock->draw(map_surface, cx, cy);
				}
				break;

				case TILES::PLATE:
				{
					sPlate->draw(map_surface, cx, cy);
				}
				break;

				case TILES::BROKEN_BLOCK:
				{
					//sBlock->draw(map_surface, cx, cy);
				}
				break;

				case TILES::WOODBOX:
				{
					//sBlock->draw(map_surface, cx, cy);
				}
				break;

				case TILES::WOODROOTBOX:
				{
					//sBlock->draw(map_surface, cx, cy);
				}
				break;

				case TILES::GATE:
				{
					//sBlock->draw(map_surface, cx, cy);
				}
				break;

				default:
					break;
			}
		}
		++i;
	}
	Render::draw_end(map_surface, m_oldhBR, m_hBR);

	Render::draw_end(map_surface, m_oldhPen, m_hPen);
}

void GameWorldMesh::clear() {
	for (int j = 0; j < TILE_JMAX; ++j) {
		if (build_backtile[j])
			delete[] build_backtile[j];
		if (build_doodads[j])
			delete[] build_doodads[j];
		if (build_instances[j])
			delete[] build_instances[j];
	}

	if (build_backtile)
		delete[] build_backtile;
	build_terrain.clear();
	if (build_doodads)
		delete[] build_doodads;
	if (build_instances)
		delete[] build_instances;
}

void GameWorldMesh::reset() {
	clear();
	build();
}

MeshPiece* GameWorldMesh::get_terrain(int ix, int iy) const {
	if (0 <= ix && 0 <= iy && ix < TILE_IMAX && iy < TILE_JMAX) {
		return build_terrain.at(ix + iy * TILE_IMAX);
	} else {
		return nullptr;
	}
}

MeshPiece* GameWorldMesh::place_terrain(double cx, double cy) {
	int tx = floor(cx / 16);
	int ty = floor(cy / 16);
	return get_terrain(tx, ty);
}

bool GameWorldMesh::place_free(double cx, double cy) {
	int tx = floor(cx / 16);
	int ty = floor(cy / 16);
	int check = get_terrain(tx, ty)->data;
	return (check != TILES::BLOCK);
}

bool GameWorldMesh::place_throughable(double cx, double cy) {
	int tx = floor(cx / 16);
	int ty = floor(cy / 16);
	int check = get_terrain(tx, ty)->data;
	return (check == TILES::PLATE);
}

bool GameWorldMesh::place_collider(double cx, double cy) {
	return place_free(cx, cy) || place_throughable(cx, cy);
}

void GameWorldMesh::on_render(HDC canvas) {
	AlphaBlend(canvas, 0, 0, VIEW_W, VIEW_H, map_surface, 0, 0, VIEW_W, VIEW_H, map_blend);
}

sceneGame::sceneGame()
	: worldmesh(this) {}

void sceneGame::on_create() {
	framework.background_color = COLOR_GREY;

	worldmesh.load("map.txt");
	worldmesh.build();
	GameScene::on_create();
}

void sceneGame::on_destroy() {
	GameScene::on_destroy();
}

void sceneGame::on_update(double frame_advance) {
	GameScene::on_update(frame_advance);
}

void sceneGame::on_render(HDC canvas) {
	worldmesh.on_render(canvas);
	GameScene::on_render(canvas);
}

MeshPiece::MeshPiece() : data('0') {}

MeshPiece::MeshPiece(const char ch) {
	data = ch;
}

MeshPiece::operator char() const {
	return data;
}

MeshPiece& MeshPiece::operator=(const char ch) {
	data = ch;
	return *this;
}

bool MeshPiece::operator==(const char ch) const {
	return (data == ch);
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
