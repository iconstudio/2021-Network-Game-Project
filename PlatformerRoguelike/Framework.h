#pragma once
#include "Sprite.h"
#include "Behavior.h"


class GameFramework {
public:
	GameFramework();
	~GameFramework();

	void on_create(); // 상태 시작
	void on_destroy(); // 상태 끝
	void on_update(double frame_advance);
	void on_mousedown(WPARAM button, LPARAM cursor);
	void on_mouseup(WPARAM button, LPARAM cursor);
	void on_keydown(WPARAM key);
	void on_keyup(WPARAM key);
	void on_render(HWND window);

	void init();
	void update();

	void delta_start();
	void delta_inspect();
	double get_elapsed_second() const;

	void input_register(WPARAM virtual_button);
	bool input_check(WPARAM virtual_button);
	bool input_check_pressed(WPARAM virtual_button);

	shared_ptr<GameSprite> make_sprite(HINSTANCE instance, UINT resource, UINT number, int xoff, int yoff);
	shared_ptr<GameSprite> make_sprite(LPCTSTR path, UINT number, int xoff, int yoff);

	template<class GScene>
	GameScene* make_scene() {
		auto room = new GScene();
		states.push_back(room);

		return room;
	}

	bool state_is_done() const;
	void state_jump(u_int index);
	void state_jump_next();

	GameScene* state_id; // 현재 상태 포인터
	u_int state_handle = 0;

	LONG mouse_x, mouse_y; // 마우스 좌표
	COLORREF background_color = COLOR_WHITE;
private:
	class GameInput {
	public:
		int time = -1;

		void on_press() { time++; }
		void on_release() { time = -1; }
		bool is_pressing() const { return (0 <= time); }
		bool is_pressed() const { return (0 == time); }
	};

	using tick_type = std::chrono::microseconds;
	using clock_type = std::chrono::system_clock::time_point;

	clock_type clock_previos, clock_now;
	LONGLONG elapsed;
	double delta_time;

	PAINTSTRUCT painter;

	vector<GameScene*> states; // 게임 장면 모음
	vector<shared_ptr<GameSprite>> sprites;
	map<WPARAM, GameInput*> key_checkers;
};