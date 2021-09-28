#include "pch.h"
#include "Framework.h"
#include "Behavior.h"

GameFramework::GameFramework()
	: mouse_x(0), mouse_y(0), delta_time(0.0), state_id(nullptr), painter{}, elapsed(0) {}

GameFramework::~GameFramework() {
	for (auto& sprite : sprites)
		sprite.reset();

	sprites.clear();
}

bool GameFramework::state_is_done() const {
	if (state_id)
		return (state_id->done);
	else
		return true;
}

void GameFramework::state_jump(u_int index) {
	auto target = states.at(index);

	if (target && target != state_id) {
		if (state_id) {
			state_id->reset();
			state_id->on_destroy();
		}

		target->done = false;
		state_id = target;
		state_handle = index;
		state_id->on_create();
	}
}

void GameFramework::state_jump_next() {
	state_jump(state_handle + 1);
}

void GameFramework::on_create() {
	if (state_id) {
		state_id->on_create();
	}
}

void GameFramework::on_destroy() {
	if (state_id) {
		state_id->on_destroy();
	}
}

void GameFramework::on_update(const double frame_advance) {
	if (state_id) {
		state_id->on_update(frame_advance);
	}
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

void GameFramework::on_render(HWND hwnd) {
	HDC surface_app = BeginPaint(hwnd, &painter);
	HDC surface_double = CreateCompatibleDC(surface_app);

	// 메인 DC 그리기 부분
	auto m_hBit = CreateCompatibleBitmap(surface_app, VIEW_W, VIEW_H);
	auto m_oldhBit = (HBITMAP)SelectObject(surface_double, m_hBit);

	// 초기화
	auto m_hPen = CreatePen(PS_NULL, 1, background_color);
	auto m_oldhPen = (HPEN)SelectObject(surface_double, m_hPen);
	auto m_hBR = CreateSolidBrush(background_color);
	auto m_oldhBR = (HBRUSH)SelectObject(surface_double, m_hBR);
	Render::draw_rectangle(surface_double, 0, 0, VIEW_W, VIEW_H);
	Render::draw_end(surface_double, m_oldhBR, m_hBR);
	Render::draw_end(surface_double, m_oldhPen, m_hPen);

	HDC surface_back = CreateCompatibleDC(surface_app);
	HBITMAP m_newBit = CreateCompatibleBitmap(surface_app, PORT_W, PORT_H);
	HBITMAP m_newoldBit = (HBITMAP)SelectObject(surface_back, m_newBit);

	// 파이프라인
	if (state_id) {
		state_id->on_render(surface_double);
	}

	// 이중 버퍼 -> 백 버퍼
	BitBlt(surface_back, 0, 0, VIEW_W, VIEW_H, surface_double, 0, 0, SRCCOPY);
	Render::draw_end(surface_double, m_oldhBit, m_hBit);

	// 백 버퍼 -> 화면 버퍼
	StretchBlt(surface_app, 0, 0, PORT_W, PORT_H, surface_back
			   , 0, 0, VIEW_W, VIEW_H, SRCCOPY);
	Render::draw_end(surface_back, m_newoldBit, m_newBit);

	DeleteDC(surface_back);
	DeleteDC(surface_double);
	ReleaseDC(hwnd, surface_app);
	EndPaint(hwnd, &painter);
}

void GameFramework::init() {
	input_register(MK_LBUTTON);
	input_register(MK_RBUTTON);
	input_register(MK_MBUTTON);
	input_register(VK_ESCAPE);
	delta_start();

	if (!states.empty())
		state_jump(0);
}

void GameFramework::update() {
	if (state_is_done())
		return;

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
	on_update(delta_time);
	delta_start();
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

shared_ptr<GameSprite> GameFramework::make_sprite(HINSTANCE instance, const UINT resource, const UINT number, const int xoff, const int yoff) {
	auto spr = make_shared<GameSprite>(instance, resource, number, xoff, yoff);
	sprites.emplace_back(spr);
	return spr;
}

shared_ptr<GameSprite> GameFramework::make_sprite(const LPCTSTR path, const UINT number, const int xoff, const int yoff) {
	auto spr = make_shared<GameSprite>(path, number, xoff, yoff);
	sprites.emplace_back(spr);
	return spr;
}
