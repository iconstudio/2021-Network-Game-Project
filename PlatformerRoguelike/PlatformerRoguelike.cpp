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
auto sBullet = framework.make_sprite(TEXT("Resources\\bullet.png"), 1, 2, 2);

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

	framework.make_scene<roomStart>();
	framework.input_register(VK_LEFT);
	framework.input_register(VK_RIGHT);
	framework.input_register(VK_UP);
	framework.input_register(VK_DOWN);
	framework.input_register(VK_SPACE);
	framework.input_register(VK_RETURN);
	framework.input_register('R');
	framework.input_register('Z');
	framework.input_register('X');

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

oGraviton::oGraviton(double x, double y, GameWorldMesh* newmesh)
	: GameInstance(x, y, newmesh), system(nullptr) {
	gravity = GRAVITY;
}

void oGraviton::jump(double power) {
	vspeed = -power;
}

oPlayer::oPlayer(double x, double y, GameWorldMesh* newmesh)
	: oGraviton(x, y, newmesh), imxs(StaticDir::RIGHT)
	, attack_cooldown(0.0), attack_combo_time(0.0), attack_count(0) {
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

	bool check_attack = framework.input_check_pressed('Z');

	int check_hor = (check_right - check_left);

	hspeed = check_hor * 10.0;

	if (-1 == check_hor) {
		imxs = StaticDir::LEFT;
	} else if (1 == check_hor) {
		imxs = StaticDir::RIGHT;
	}

	if (check_jump) {
		jump(PLAYER_JUMP_VELOCITY);
	}
	
	if (check_attack && attack_cooldown <= 0.0) {
		attack_count = attack_count_max;
	}

	if (0 < attack_combo_time) {
		attack_combo_time -= frame_advance;
	} else if (0 < attack_count) {
		double shdir = (int)imxs;
		double shx = x + shdir * 3;
		auto shoot = room->instance_create<oPlayerBullet>(shx, y - 2, worldmesh);
		shoot->hspeed = BULLET_VELOCITY * shdir;

		if (0 < attack_count) {
			attack_combo_time = attack_combo_period;
			attack_count--;
		} else {
			attack_cooldown = attack_period;
		}
	} else if (0 < attack_cooldown) {
		attack_cooldown -= frame_advance;
	}

	oGraviton::on_update(frame_advance);
}

oPlayerBullet::oPlayerBullet(double x, double y, GameWorldMesh* newmesh)
	: oGraviton(x, y, newmesh) {
	set_sprite(sBullet);
	gravity = 0.0;
}

GameWorldMesh::GameWorldMesh(roomStart* room)
	: my_room(room), map_bitmap(NULL), map_surface(NULL) {
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
				build_terrain.push_back(new GameMeshPiece(ch));
				++j;
			}
			//fread(build_terrain[j], sizeof(char), TILE_IMAX, myfile);
		}

		if (j < build_terrain.size())
			throw std::exception("Invalid map file.");

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

		if (data != TILES::NONE) {
			cx = 16.0 * (i - floor(i / TILE_IMAX) * TILE_IMAX);
			cy = 16.0 * floor(i / TILE_IMAX);

			switch (data) {
				case TILES::PLAYER:
				{
					auto pp = my_room->instance_create<oPlayer>(cx + 8.0, cy + 8.0, this);
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
	build_backtile.clear();
	build_terrain.clear();
	build_doodads.clear();
	build_instances.clear();
}

void GameWorldMesh::reset() {
	clear();
	build();
}

GameMeshPiece* GameWorldMesh::get_terrain(int ix, int iy) const {
	if (0 <= ix && 0 <= iy && ix < TILE_IMAX && iy < TILE_JMAX) {
		return build_terrain.at(ix + iy * TILE_IMAX);
	} else {
		return nullptr;
	}
}

GameMeshPiece* GameWorldMesh::place_terrain(double cx, double cy) {
	int tx = floor(cx / 16);
	int ty = floor(cy / 16);
	return get_terrain(tx, ty);
}

bool GameWorldMesh::place_free(double cx, double cy) {
	char check = place_terrain(cx, cy)->data;
	return (check != TILES::BLOCK);
}

bool GameWorldMesh::place_throughable(double cx, double cy) {
	char check = place_terrain(cx, cy)->data;
	return (check == TILES::PLATE);
}

bool GameWorldMesh::place_collider(double cx, double cy) {
	return place_free(cx, cy) || place_throughable(cx, cy);
}

void GameWorldMesh::on_render(HDC canvas) {
	AlphaBlend(canvas, 0, 0, VIEW_W, VIEW_H, map_surface, 0, 0, VIEW_W, VIEW_H, map_blend);
}

roomStart::roomStart()
	: worldmesh(this) {}

void roomStart::on_create() {
	framework.background_color = COLOR_GREY;

	worldmesh.load("map.txt");
	worldmesh.build();
	GameScene::on_create();
}

void roomStart::on_destroy() {
	GameScene::on_destroy();
}

void roomStart::on_update(double frame_advance) {
	GameScene::on_update(frame_advance);
}

void roomStart::on_render(HDC canvas) {
	worldmesh.on_render(canvas);
	GameScene::on_render(canvas);
}

GameMeshPiece::GameMeshPiece() : data('0') {}

GameMeshPiece::GameMeshPiece(const char ch) {
	data = ch;
}

GameMeshPiece::operator char() const {
	return data;
}

GameMeshPiece& GameMeshPiece::operator=(const char ch) {
	data = ch;
	return *this;
}

bool GameMeshPiece::operator==(const char ch) const {
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
