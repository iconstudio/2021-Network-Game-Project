#include "pch.h"
#include "PlatformerRoguelike.h"
#include "Framework.h"

#define MAX_LOADSTRING 100
#define RENDER_TIMER_ID 1

WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
WindowsClient game_client{ CLIENT_W, CLIENT_H };
GameFramework framework{ VIEW_W, VIEW_H, PORT_W, PORT_H };

static unsigned long long score = 0;
static unsigned long long lives = 0;
static unsigned long long shield = 0;

auto sBall = framework.make_sprite(TEXT("Resources\\ball.png"), 1, 32, 32);
auto sBlock = framework.make_sprite(TEXT("Resources\\blockpurple.png"), 1, 0, 0);
auto sPlate = framework.make_sprite(TEXT("Resources\\plate.png"), 1, 0, 0);
auto sPlayer = framework.make_sprite(TEXT("Resources\\player.png"), 1, 6, 6);
auto sPlayerLeft = framework.make_sprite(TEXT("Resources\\player_left.png"), 1, 6, 6);
auto sBullet = framework.make_sprite(TEXT("Resources\\bullet.png"), 1, 2, 2);
auto sWoodbox = framework.make_sprite(TEXT("Resources\\box.png"), 1, 8, 8);
auto sDoor = framework.make_sprite(TEXT("Resources\\door.png"), 1, 0, 0);
auto sClosedDoor = framework.make_sprite(TEXT("Resources\\door_closed.png"), 1, 0, 0);
auto sBacktile = framework.make_sprite(TEXT("Resources\\background.png"), 1, 0, 0);

LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_PLATFORMERROGUELIKE, szWindowClass, MAX_LOADSTRING);

	if (!game_client.initialize(hInstance, WndProc, szTitle, szWindowClass, nCmdShow))
	{
		return FALSE;
	}

	framework.make_scene<roomStart>();
	framework.input_register(VK_RETURN);
	framework.input_register(VK_LEFT);
	framework.input_register(VK_RIGHT);
	framework.input_register(VK_UP);
	framework.input_register(VK_DOWN);
	framework.input_register(VK_SPACE);
	framework.input_register('R');
	framework.input_register('Z');
	framework.input_register('X');
	framework.input_register('C');

	framework.input_register(VK_F1); // 도움말
	framework.input_register(VK_F2); // 게임 전체 다시 시작
	framework.input_register(VK_F3); // 현재 장면만 다시 시작

	framework.init();

	MSG msg;
	while (true)
	{
		if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

oPlayer::oPlayer(double x, double y, GameWorldMesh* newmesh)
	: GameInstance(x, y, newmesh), imxs(1)
	, attack_cooldown(0.0), attack_combo_time(0.0), attack_count(0)
{
	set_sprite(sPlayer);
	gravity = GRAVITY;
}

void oPlayer::on_create()
{
	attack_count = 0;
	GameInstance::on_create();
}

void oPlayer::on_update(double frame_advance)
{
	int check_left = framework.input_check(VK_LEFT);
	int check_right = framework.input_check(VK_RIGHT);
	int check_up = framework.input_check(VK_UP);
	int check_down = framework.input_check(VK_DOWN);

	bool check_jump = framework.input_check_pressed(VK_SPACE);
	bool check_jump_cnt = framework.input_check(VK_SPACE);

	bool check_attack = framework.input_check_pressed('Z');

	int check_hor = (check_right - check_left);

	hspeed = check_hor * PLAYER_WALK_VELOCITY;

	if (0 != check_hor)
	{
		imxs = check_hor;
		if (imxs == -1)
			sprite_index = sPlayerLeft;
		else
			sprite_index = sPlayer;
	}

	if (!in_air)
		jumped = false;
	if (jumped && vspeed < 0.0 && !check_jump_cnt)
	{
		jumped = false;
		vspeed *= 0.5;
	}
	if (check_jump && !in_air && !jumped)
	{
		jump(PLAYER_JUMP_VELOCITY);
		jumped = true;
	}

	if (check_attack && attack_cooldown <= 0.0)
	{
		attack_count = attack_count_max;
	}

	if (0 < attack_combo_time)
	{
		attack_combo_time -= frame_advance;
	}
	else if (0 < attack_count)
	{
		if (check_up)
		{
			auto shoot = my_room->instance_create<oPlayerBullet>(x, bbox_top() + 1, worldmesh);
			shoot->vspeed = -BULLET_VELOCITY;
			shoot->image_angle = 90.0;
		}
		else
		{
			double shx = x + imxs * 3.0;
			auto shoot = my_room->instance_create<oPlayerBullet>(shx, y, worldmesh);
			shoot->hspeed = BULLET_VELOCITY * imxs;
			shoot->image_angle = imxs * 180.0;
		}

		if (0 < attack_count)
		{
			attack_combo_time = attack_combo_period;
			attack_count--;
		}
		else
		{
			attack_cooldown = attack_period;
		}
	}
	else if (0 < attack_cooldown)
	{
		attack_cooldown -= frame_advance;
	}

	GameInstance::on_update(frame_advance);
}

void oPlayer::thud()
{
	jumped = false;
	GameInstance::thud();
}

oPlayerBullet::oPlayerBullet(double x, double y, GameWorldMesh* newmesh)
	: GameInstance(x, y, newmesh), time(0.0)
{
	set_sprite(sBullet);
}

void oPlayerBullet::on_update(double frame_advance)
{
	if (time < BULLET_EXPIRE_PERIOD)
	{
		time += frame_advance;
	}
	else
	{
		my_room->instance_kill(this);
		return;
	}
	GameInstance::on_update(frame_advance);
}

void oPlayerBullet::thud()
{
	my_room->instance_kill(this);
}

void oPlayerBullet::ceil()
{
	my_room->instance_kill(this);
}

void oPlayerBullet::side()
{
	my_room->instance_kill(this);
}

oDoor::oDoor(double x, double y)
	: GameInstance(x, y), closed(false)
{
	set_sprite(sDoor);
}

void oDoor::close()
{
	closed = true;
	set_sprite(sClosedDoor);
}

oWoodbox::oWoodbox(double x, double y, GameWorldMesh* newmesh)
	: GameInstance(x, y, newmesh)
{
	set_sprite(sWoodbox);
}

roomStart::roomStart() : GameScene(VIEW_W, VIEW_H)
{
	fade_blender.BlendOp = AC_SRC_OVER;
	fade_blender.BlendFlags = 0;
	fade_blender.AlphaFormat = 0; //AC_SRC_ALPHA;
}

void roomStart::on_create()
{
	score = 0;
	lives = 5;
	shield = 0;
	complete = false;
	start_time = 0.0;
	clear_time = 0.0;

	framework.background_color = COLOR_GREY;

	HDC world_dc = GetDC(NULL);
	cutout_surface = CreateCompatibleDC(world_dc);
	cutout_bitmap = CreateCompatibleBitmap(world_dc, VIEW_W, VIEW_H);
	SelectObject(cutout_surface, cutout_bitmap);
	fade_surface = CreateCompatibleDC(world_dc);
	fade_bitmap = CreateCompatibleBitmap(world_dc, VIEW_W, VIEW_H);
	SelectObject(fade_surface, fade_bitmap);
	Render::draw_clear(fade_surface, VIEW_W, VIEW_H, 0);

	worldmesh.load("map_start.txt");
	worldmesh.build();
	GameScene::on_create();
}

void roomStart::on_destroy()
{
	DeleteDC(cutout_surface);
	DeleteObject(cutout_bitmap);
	DeleteDC(fade_surface);
	DeleteObject(fade_bitmap);
	GameScene::on_destroy();
}

void roomStart::on_update(double frame_advance)
{
	if (framework.input_check_pressed(VK_F3))
	{
		reset();
		return;
	}

	if (complete)
	{
		if (clear_time < CLEAR_WAIT_PERIOD)
		{
			clear_time += frame_advance;
		}
		else
		{
			done = true;
		}
	}
	else if (start_time < START_WAIT_PERIOD)
	{
		start_time += frame_advance;
	}
	else
	{
		GameScene::on_update(frame_advance);
	}
}

void roomStart::on_render(HDC canvas)
{
	worldmesh.on_render(canvas);
	GameScene::on_render(canvas);

	double fade_rate = 0.0;
	double fade_cx, fade_cy, fade_ex1, fade_ey1, fade_ex2, fade_ey2;

	if (complete)
	{
		if (clear_time < CLEAR_WAIT_PERIOD)
		{
			fade_rate = min(1.0, clear_time / CLEAR_WAIT_PERIOD);
		}
		else
		{
			fade_rate = 1.0;
		}
		fade_cx = VIEW_W * 0.5;
		fade_cy = VIEW_H * 0.5;
	}
	else if (start_time < START_WAIT_PERIOD)
	{
		fade_rate = min(1.0, start_time / START_WAIT_PERIOD);
		fade_cx = worldmesh.playerx;
		fade_cy = worldmesh.playery;
	}

	if (0.0 < fade_rate)
	{
		Render::draw_clear(cutout_surface, VIEW_W, VIEW_H, 0);

		auto m_hPen = CreatePen(PS_NULL, 1, COLOR_GOLD);
		auto m_oldhPen = (HPEN)SelectObject(cutout_surface, m_hPen);
		auto m_hBR = CreateSolidBrush(COLOR_GOLD);
		auto m_oldhBR = (HBRUSH)SelectObject(cutout_surface, m_hBR);

		double fade_width = fade_rate * VIEW_W;
		fade_ex1 = fade_cx - fade_width;
		fade_ey1 = fade_cy - fade_width;
		fade_ex2 = fade_cx + fade_width;
		fade_ey2 = fade_cy + fade_width;
		Ellipse(cutout_surface, fade_ex1, fade_ey1, fade_ex2, fade_ey2);

		Render::draw_end(cutout_surface, m_oldhPen, m_hPen);
		Render::draw_end(cutout_surface, m_oldhBR, m_hBR);

		m_hPen = CreatePen(PS_NULL, 1, COLOR_BLACK);
		m_oldhPen = (HPEN)SelectObject(cutout_surface, m_hPen);
		m_hBR = CreateSolidBrush(COLOR_BLACK);
		m_oldhBR = (HBRUSH)SelectObject(fade_surface, m_hBR);
		Rectangle(fade_surface, 0, 0, VIEW_W, VIEW_H);
		Render::draw_end(fade_surface, m_oldhPen, m_hPen);
		Render::draw_end(fade_surface, m_oldhBR, m_hBR);

		fade_blender.SourceConstantAlpha = 255 * ((1.0 - pow(fade_rate, 1.2)));
		TransparentBlt(canvas, 0, 0, VIEW_W, VIEW_H, cutout_surface, 0, 0, VIEW_W, VIEW_H, COLOR_GOLD);
		AlphaBlend(canvas, 0, 0, VIEW_W, VIEW_H, fade_surface, 0, 0, VIEW_W, VIEW_H, fade_blender);
	}
}

roomGame::roomGame() : GameScene(VIEW_W * 3, 640)
{

}

void roomGame::on_create()
{
	view_x = worldmesh.playerx - VIEW_W * 0.5;
	view_y = worldmesh.playery - VIEW_H * 0.5;
}

void roomGame::on_destroy()
{

}

void roomGame::on_update(double frame_advance)
{

}

void roomGame::on_render(HDC canvas)
{

}

GameMeshPiece::GameMeshPiece(const char ch) : data(ch) {}

GameSpawnPoint::GameSpawnPoint(SPAWN_TYPES ctype, int cx, int cy)
	: type(ctype), x(cx), y(cy), difficulty(SPAWN_DIFFICULTY::NO) {}

GameWorldMesh::GameWorldMesh(GameScene* room, int new_width, int new_height)
	: my_room(room), map_bitmap(NULL), map_surface(NULL)
	, width(new_width), height(new_height), iwidth(new_width / 16), iheight(new_width / 16)
{}

GameWorldMesh::~GameWorldMesh()
{
	DeleteDC(map_surface);
	DeleteObject(map_bitmap);
	clear();
}

void GameWorldMesh::load(const char* mapfile)
{
	build_terrain.clear();
	build_terrain.reserve(iheight * iwidth + 1);

	FILE* myfile = nullptr;
	auto result = fopen_s(&myfile, mapfile, "r");
	if (myfile)
	{
		u_int ch, j = 0;
		while (!feof(myfile))
		{
			ch = fgetc(myfile);

			if (ch != '\n')
			{
				build_terrain.push_back(new GameMeshPiece(ch));
				++j;
			}
		}

		fclose(myfile);
	}
	else
	{
		if (MessageBox(NULL, TEXT("게임 지도 파일을 찾을 수 없습니다."), TEXT("오류"), MB_OK | MB_ICONERROR))
		{
			DestroyWindow(game_client.hwindow);
			return;
		}
	}
}

void GameWorldMesh::set_tile(int ix, int iy, TILES tile)
{

}

void GameWorldMesh::make_tile(int ix, int iy)
{

}

void GameWorldMesh::build()
{
	HDC world_dc = GetDC(NULL);
	map_surface = CreateCompatibleDC(world_dc);
	map_bitmap = CreateCompatibleBitmap(world_dc, VIEW_W, VIEW_H);
	SelectObject(map_surface, map_bitmap);
	Render::draw_clear(map_surface, VIEW_W, VIEW_H, 0);

	auto m_hPen = CreatePen(PS_NULL, 1, COLOR_GREEN);
	auto m_oldhPen = (HPEN)SelectObject(map_surface, m_hPen);
	auto m_hBR = CreateSolidBrush(COLOR_GREEN);
	auto m_oldhBR = (HBRUSH)SelectObject(map_surface, m_hBR);

	double cx, cy;

	for (int j = 0; j < 10; ++j)
	{
		for (int k = 0; k < 8; ++k)
		{
			cx = j * 32.0;
			cy = k * 32.0;
			sBacktile->draw(map_surface, cx, cy);
		}
	}

	int i = 0;
	for (const auto& tile : build_terrain)
	{
		const char data = tile->data;

		if (data != TILES::NONE)
		{
			cx = 16.0 * (i - floor(i / iwidth) * iwidth);
			cy = 16.0 * floor(i / iwidth);

			switch (data)
			{
				case TILES::PLAYER:
				{
					playerx = cx + 8.0;
					playery = cy + 8.0;
					auto pp = my_room->instance_create<oPlayer>(playerx, playery, this);
					pp->raycast_dw(16.0);
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
					//
				}
				break;

				case TILES::WOODBOX:
				{
					my_room->instance_create<oWoodbox>(cx + 8.0, cy + 8.0, this);
				}
				break;

				case TILES::WOODROOTBOX:
				{
					my_room->instance_create<oWoodbox>(cx + 8.0, cy + 8.0, this);
				}
				break;

				case TILES::GATE:
				{
					my_room->instance_create<oDoor>(cx, cy);
				}
				break;

				case TILES::SPAWN:
				{
					spawns.push_back(new GameSpawnPoint(SPAWN_TYPES::NORMAL, cx, cy));
				}
				break;

				case TILES::AIRSPAWN:
				{
					//
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

void GameWorldMesh::clear()
{
	build_backtile.clear();
	build_terrain.clear();
	build_doodads.clear();
	build_instances.clear();
}

void GameWorldMesh::reset()
{
	clear();
	my_room->reset();
}

GameMeshPiece* GameWorldMesh::get_terrain(int ix, int iy) const
{
	if (0 <= ix && 0 <= iy && ix < iwidth && iy < iheight)
	{
		u_int index = ix + iy * iwidth;
		if (index < build_terrain.size())
			return build_terrain.at(index);
		else
			return nullptr;
	}
	else
	{
		return nullptr;
	}
}

GameMeshPiece* GameWorldMesh::place_terrain(double cx, double cy)
{
	int tx = floor(cx / 16);
	int ty = floor(cy / 16);
	return get_terrain(tx, ty);
}

bool GameWorldMesh::place_solid(double cx, double cy)
{
	auto check = place_terrain(cx, cy);
	if (check)
	{
		char data = check->data;
		return (data == TILES::BLOCK);
	}
	else
	{
		return false;
	}
}

bool GameWorldMesh::place_plate(double cx, double cy)
{
	auto check = place_terrain(cx, cy);
	if (check)
	{
		char data = check->data;
		return (data == TILES::PLATE);
	}
	else
	{
		return false;
	}
}

bool GameWorldMesh::place_collider(double cx, double cy)
{
	return place_solid(cx, cy) || place_plate(cx, cy);
}

void GameWorldMesh::on_render(HDC canvas)
{
	BitBlt(canvas, 0, 0, VIEW_W, VIEW_H, map_surface, 0, 0, SRCCOPY);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
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
