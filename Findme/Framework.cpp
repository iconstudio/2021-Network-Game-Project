#include "stdafx.h"
#include "Framework.h"


GameInstance::GameInstance(char** mesh)
	: worldmesh(mesh), box{}, dead(false)
	, x(0), y(0), hspeed(0.0), vspeed(0.0) {}

GameInstance::~GameInstance() {
	if (sprite_index)
		sprite_index.reset();
}

void GameInstance::on_create() {}

void GameInstance::on_destroy() {}

void GameInstance::on_update(double frame_advance) {
	if (!worldmesh) {
		x += hspeed * frame_advance;
		y += vspeed * frame_advance;
	} else {
		if (0 != hspeed) {
			double cast_x, distance_x = abs(hspeed) * frame_advance;

			if (0 < hspeed) {
				cast_x = raycast_rt(distance_x);
			} else {
				cast_x = raycast_lt(distance_x);
			}

			if (0 < cast_x) {
			}
		}

		double cast_y, distance_y = abs(vspeed) * frame_advance;
		if (vspeed < 0) { // upward
			cast_y = raycast_up(distance_y);
		} else {
			cast_y = raycast_dw(distance_y);
		}

		if (0 < cast_y) {
			if (0 < vspeed) {
			} else {
			}
		}
	}
}

void GameInstance::on_render(HDC canvas) {
	if (sprite_index) {
		sprite_index->draw(canvas, x, y, 0.0, 0.0, 1.0, 1.0, 1.0);
	}
}

void GameInstance::set_sprite(shared_ptr<GameSprite>& sprite) {
	sprite_index = sprite;
	CopyRect(&box, &(sprite->bbox));
}

int GameInstance::bbox_left() const {
	return x + box.left;
}

int GameInstance::bbox_top() const {
	return y + box.top;
}

int GameInstance::bbox_right() const {
	return x + box.right;
}

int GameInstance::bbox_bottom() const {
	return y + box.bottom;
}

char GameInstance::get_terrain(int ix, int iy) const {
	if (0 <= ix && 0 <= iy && ix < (GAME_SCENE_W / 16) && iy < (GAME_SCENE_H / 16)) {
		return worldmesh[iy][ix];
	} else {
		return -1;
	}
}

char GameInstance::place_terrain(double cx, double cy) {
	int tx = floor(cx / 16);
	int ty = floor(cy / 16);
	return get_terrain(tx, ty);
}

bool GameInstance::place_solid(double cx, double cy) {
	auto data = place_terrain(cx, cy);
	if (data) {
		return (data == '#');
	} else {
		return false;
	}
}

bool GameInstance::collide_with(GameInstance*& other) {
	return !(other->bbox_right() < bbox_left() || other->bbox_bottom() < bbox_top()
			|| bbox_right() < other->bbox_left() || bbox_bottom() < other->bbox_top());
}

int GameInstance::raycast_lt(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (false) {
			break;
		}
		x--;
		move_distance--;
	}

	return move_distance;
}

int GameInstance::raycast_rt(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (false) {
			break;
		}
		x++;
		move_distance--;
	}

	return move_distance;
}

int GameInstance::raycast_up(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (false) {
			break;
		}
		y--;
		move_distance--;
	}

	return move_distance;
}

int GameInstance::raycast_dw(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (false) {
			break;
		}
		y++;
		move_distance--;
	}

	return move_distance;
}

GameFramework::GameFramework(int rw, int rh, int vw, int vh, int pw, int ph)
	: mouse_x(0), mouse_y(0), delta_time(0.0), painter{}, elapsed(0)
	, scene_width(rw), scene_height(rh)
	, view_width(vw), view_height(vh), port_width(pw), port_height(ph) {

	screen_x = (CLIENT_W - port_width) * 0.5;
	screen_y = 0;//(CLIENT_H - port_height) * 0.25;
}

GameFramework::~GameFramework() {}

void GameFramework::init() {
	input_register(MK_LBUTTON);
	input_register(MK_RBUTTON);
	input_register(MK_MBUTTON);
	input_register(VK_ESCAPE);
	delta_start();
}

void GameFramework::update() {
	delta_inspect();

	for (auto& key_pair : key_checkers) {
		short check = GetAsyncKeyState(key_pair.first);

		auto state = key_pair.second;

		if (HIBYTE(check) == 0) { // released
			state->on_release();
		} else if (check & 0x8000) {
			state->on_press();
		}
	}

	delta_time = get_elapsed_second();
	for_each_instances([&](GameInstance*& inst) {
		inst->on_update(delta_time);
	});
	delta_start();
}

void GameFramework::draw(HWND window) {
	HDC surface_app = BeginPaint(window, &painter);

	HDC surface_double = CreateCompatibleDC(surface_app);
	HBITMAP m_hBit = CreateCompatibleBitmap(surface_app, view_width, view_height);
	HBITMAP m_oldhBit = (HBITMAP)SelectObject(surface_double, m_hBit);

	// 초기화
	Render::draw_clear(surface_double, view_width, view_height, background_color);

	HDC surface_back = CreateCompatibleDC(surface_app);
	HBITMAP m_newBit = CreateCompatibleBitmap(surface_app, view_width, view_height);
	HBITMAP m_newoldBit = (HBITMAP)SelectObject(surface_back, m_newBit);

	// 파이프라인
	for_each_instances([&](GameInstance*& inst) {
		inst->on_render(surface_double);
	});

	// 이중 버퍼 -> 백 버퍼
	BitBlt(surface_back, 0, 0, view_width, view_height, surface_double, 0, 0, SRCCOPY);
	Render::draw_end(surface_double, m_oldhBit, m_hBit);

	// 백 버퍼 -> 화면 버퍼
	StretchBlt(surface_app, screen_x, screen_y, port_width, port_height
			   , surface_back, 0, 0, view_width, view_height, SRCCOPY);
	Render::draw_end(surface_back, m_newoldBit, m_newBit);

	DeleteDC(surface_back);
	DeleteDC(surface_double);
	ReleaseDC(window, surface_app);
	EndPaint(window, &painter);
}

void GameFramework::on_mousedown(const WPARAM button, const LPARAM cursor) {
	auto vk_status = key_checkers[button];
	vk_status->on_press();

	mouse_x = LOWORD(cursor);
	mouse_y = HIWORD(cursor);
}

void GameFramework::on_mouseup(const WPARAM button, const LPARAM cursor) {
	auto vk_status = key_checkers[button];
	vk_status->on_release();

	mouse_x = LOWORD(cursor);
	mouse_y = HIWORD(cursor);
}

void GameFramework::on_keydown(const WPARAM key) {
	auto vk_status = key_checkers.find(key);
	if (vk_status != key_checkers.end()) {
		vk_status->second->on_press();
	}
}

void GameFramework::on_keyup(const WPARAM key) {
	auto vk_status = key_checkers.find(key);
	if (vk_status != key_checkers.end()) {
		vk_status->second->on_release();
	}
}

void GameFramework::delta_start() {
	clock_previos = std::chrono::system_clock::now();
}

void GameFramework::delta_inspect() {
	clock_now = std::chrono::system_clock::now();

	elapsed = std::chrono::duration_cast<tick_type>(clock_now - clock_previos).count();
}

double GameFramework::get_elapsed_second() const {
	return ((double)elapsed / (double)tick_type::period::den);
}

void GameFramework::input_register(const WPARAM virtual_button) {
	key_checkers.emplace(virtual_button, new GameInput());
}

bool GameFramework::input_check(const WPARAM virtual_button) {
	auto checker = key_checkers.find(virtual_button);
	if (checker != key_checkers.end()) {
		return checker->second->is_pressing();
	}

	return false;
}

bool GameFramework::input_check_pressed(const WPARAM virtual_button) {
	auto checker = key_checkers.find(virtual_button);
	if (checker != key_checkers.end()) {
		return checker->second->is_pressed();
	}

	return false;
}

void GameFramework::set_mesh(char**& new_mesh) {
	worldmesh = new_mesh;
}
