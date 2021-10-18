#include "stdafx.h"
#include "Framework.h"


GameInstance::GameInstance(GameMesh***& mesh)
	: worldmesh(mesh), box{}, dead(false)
	, x(0), y(0), hspeed(0.0), vspeed(0.0), xscale(1.0), yscale(1.0) {}

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
		sprite_index->draw(canvas, x, y, 0.0, 0.0, xscale, yscale, 1.0);
	}
}

void GameInstance::set_sprite(shared_ptr<GameSprite>& sprite) {
	sprite_index = sprite;
	set_mask(sprite->bbox);
}

void GameInstance::set_mask(RECT& mask) {
	CopyRect(&box, &mask);
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

GameMesh* GameInstance::get_terrain(int ix, int iy) const {
	if (0 <= ix && 0 <= iy && ix < (GAME_SCENE_W / 16) && iy < (GAME_SCENE_H / 16)) {
		return worldmesh[iy][ix];
	} else {
		return nullptr;
	}
}

GameMesh* GameInstance::place_terrain(double cx, double cy) {
	int tx = floor(cx / 16);
	int ty = floor(cy / 16);
	return get_terrain(tx, ty);
}

bool GameInstance::place_solid(double cx, double cy) {
	auto mesh = place_terrain(cx, cy);
	if (mesh) {
		return (mesh->data == '#');
	} else {
		return false;
	}
}

bool GameInstance::collide_with(RECT& other) {
	return !(other.right <= bbox_left() || other.bottom <= bbox_top()
			|| bbox_right() < other.left || bbox_bottom() < other.top);
}

bool GameInstance::collide_with(GameInstance*& other) {
	return !(other->bbox_right() <= bbox_left() || other->bbox_bottom() <= bbox_top()
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
	, world_w(rw), world_h(rh)
	, view{ 0, 0, vw, vh }, port{ 0, 0, pw, ph }
	, view_track_enabled(false), view_target(nullptr) {
	view.xoff = vw * 0.5;
	view.yoff = vh * 0.5;
	port.x = (CLIENT_W - pw) * 0.5;
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

	if (view_track_enabled) {
		if (view_target) {
			//view.x = max(0, min(world_w - view.w, (int)view_target->x - view.xoff));
			//view.y = max(0, min(world_h - view.h, (int)view_target->y - view.yoff));
			set_view_pos(view_target->x, view_target->y);
		}
	}

	delta_start();
}

void GameFramework::draw(HWND window) {
	HDC surface_app = BeginPaint(window, &painter);

	HDC surface_double = CreateCompatibleDC(surface_app);
	HBITMAP m_hBit = CreateCompatibleBitmap(surface_app, world_w, world_h);
	HBITMAP m_oldhBit = (HBITMAP)SelectObject(surface_double, m_hBit);

	// 초기화
	Render::draw_clear(surface_double, world_w, world_h, background_color);

	HDC surface_back = CreateCompatibleDC(surface_app);
	HBITMAP m_newBit = CreateCompatibleBitmap(surface_app, view.w, view.h);
	HBITMAP m_newoldBit = (HBITMAP)SelectObject(surface_back, m_newBit);

	// 파이프라인
	for_each_instances([&](GameInstance*& inst) {
		if (inst->sprite_index) {
			if (!(view.x + view.w <= inst->bbox_left() || inst->bbox_right() < view.x
				|| view.y + view.h <= inst->bbox_top() || inst->bbox_bottom() < view.y))
				inst->on_render(surface_double);
		} else {
			inst->on_render(surface_double);
		}
	});

	// 이중 버퍼 -> 백 버퍼
	BitBlt(surface_back, 0, 0, view.w, view.h, surface_double, view.x, view.y, SRCCOPY);
	Render::draw_end(surface_double, m_oldhBit, m_hBit);

	// 백 버퍼 -> 화면 버퍼
	StretchBlt(surface_app, port.x, port.y, port.w, port.h
			   , surface_back, 0, 0, view.w, view.h, SRCCOPY);
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

void GameFramework::set_mesh(GameMesh***& new_mesh) {
	worldmesh = new_mesh;
}

void GameFramework::set_view_tracking(bool flag) {
	view_track_enabled = flag;
}

void GameFramework::set_view_target(GameInstance* target) {
	view_target = target;
}

void GameFramework::set_view_pos(int vx, int vy) {
	view.x = max(0, min(world_w - view.w, vx - view.xoff));
	view.y = max(0, min(world_h - view.h, vy - view.yoff));
}
