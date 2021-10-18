#pragma once
#include "stdafx.h"
#include "Sprite.h"


class GameMesh : public RECT {
public:
	char data = '0';
};

class GameInstance {
public:
	GameInstance(GameInstance&) = default;
	GameInstance(GameInstance&&) = default;
	GameInstance(GameMesh***& mesh);
	virtual ~GameInstance();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	void set_sprite(shared_ptr<GameSprite>& sprite);
	void set_mask(RECT& mask);
	int bbox_left() const;
	int bbox_top() const;
	int bbox_right() const;
	int bbox_bottom() const;

	GameMesh* get_terrain(int ix, int iy) const;
	GameMesh* place_terrain(double cx, double cy);
	bool place_solid(double cx, double cy);

	bool collide_with(RECT& other);
	bool collide_with(GameInstance*& other);
	int raycast_lt(double distance);
	int raycast_rt(double distance);
	int raycast_up(double distance);
	int raycast_dw(double distance);

	GameMesh*** const worldmesh;

	shared_ptr<GameSprite> sprite_index;
	RECT box; // 충돌체
	bool dead;
	double x, y, hspeed, vspeed;
	double xscale, yscale;
};

class GameFramework {
public:
	GameFramework(int rw, int rh, int vw, int vh, int pw, int ph);
	~GameFramework();

	void init();
	void update();
	void draw(HWND window);

	void on_mousedown(WPARAM button, LPARAM cursor);
	void on_mouseup(WPARAM button, LPARAM cursor);
	void on_keydown(WPARAM key);
	void on_keyup(WPARAM key);
	void input_register(WPARAM virtual_button);
	bool input_check(WPARAM virtual_button);
	bool input_check_pressed(WPARAM virtual_button);

	void set_mesh(GameMesh***& new_mesh);
	template<class _GameClass = GameInstance>
	_GameClass* instance_create(int x = 0, int y = 0);

	LONG mouse_x, mouse_y; // 마우스 좌표
	COLORREF background_color = COLOR_WHITE;
	GameMesh*** worldmesh;
	const int world_w, world_h;

	void set_view_tracking(bool flag);
	void set_view_target(GameInstance* target);
	void set_view_pos(int vx, int vy);
	struct {
		int x, y, w, h, xoff, yoff;
	} view, port;
private:
	void delta_start();
	void delta_inspect();
	double get_elapsed_second() const;

	template<class Predicate>
	void for_each_instances(Predicate predicate);

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

	vector<GameInstance*> instances;
	map<WPARAM, GameInput*> key_checkers;
	bool view_track_enabled;
	GameInstance* view_target;
};

template<class _GameClass>
inline _GameClass* GameFramework::instance_create(int x, int y) {
	auto result = new _GameClass(worldmesh);
	result->x = x;
	result->y = y;
	result->on_create();

	instances.push_back(result);

	return result;
}

template<class Predicate>
inline void GameFramework::for_each_instances(Predicate predicate) {
	if (!instances.empty()) {
		std::for_each(instances.begin(), instances.end(), predicate);
	}
}
