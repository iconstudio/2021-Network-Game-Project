#pragma once
#include "pch.h"
#include "Sprite.h"


class GameMeshPiece {
public:
	GameMeshPiece();
	GameMeshPiece(const char ch);
	explicit operator char() const;
	GameMeshPiece& operator=(const char ch);
	bool operator==(const char ch) const;

	char data;
};

class GameWorldMesh {
public:
	GameWorldMesh(roomStart* room);
	~GameWorldMesh();

	void load(const char* mapfile);
	void build();
	void clear();
	void reset();

	GameMeshPiece* get_terrain(int ix, int iy) const;
	GameMeshPiece* place_terrain(double cx, double cy);
	bool place_free(double cx, double cy);
	bool place_throughable(double cx, double cy);
	bool place_collider(double cx, double cy);

	void on_render(HDC canvas);

private:
	roomStart* my_room;

	HDC map_surface;
	HBITMAP map_bitmap;
	BLENDFUNCTION map_blend;

	vector<GameMeshPiece*> build_instances;
	vector<GameMeshPiece*> build_doodads;
	vector<GameMeshPiece*> build_terrain;
	vector<GameMeshPiece*> build_backtile;
};

// 개체 컴포넌트
class GameInstance {
public:
	GameInstance(GameInstance&) = default;
	GameInstance(GameInstance&&) = default;
	GameInstance(double x = 0.0, double y = 0.0, GameWorldMesh* newmesh = nullptr);
	virtual ~GameInstance();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	void set_sprite(shared_ptr<GameSprite>& sprite);
	void set_mask(shared_ptr<GameSprite>& sprite);
	void set_worldmesh(GameWorldMesh* newworld);

	double bbox_left() const;
	double bbox_top() const;
	double bbox_right() const;
	double bbox_bottom() const;

	bool collide_with(GameInstance*& other);
	double raycast_lt(double distance);
	double raycast_rt(double distance);
	double raycast_up(double distance);
	double raycast_dw(double distance);

	shared_ptr<GameSprite> sprite_index; // 스프라이트
	RECT box; // 충돌체
	GameWorldMesh* worldmesh;

	double image_index, image_speed; // 애니메이션
	double image_xscale, image_yscale, image_angle, image_alpha;
	double x, y; // 좌표

	double hspeed, vspeed;
	double gravity;

	GameScene* room; // 소속 장면
};

// 장면 컴포넌트
class GameScene {
public:
	GameScene();
	virtual ~GameScene();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	void reset();

	template<class _GObj, typename ...Ty>
	auto instance_create(Ty... args) {
		_GObj* lptr = new _GObj(args...);
		lptr->room = this;

		instances.push_back(lptr);

		return lptr;
	}

	template<class _GObj>
	void instance_kill(_GObj* target) {
		auto loc = find_if(instances.begin(), instances.end(), [target](const auto& lhs) {
			return (lhs == target);
		});

		if (loc != instances.end()) {
			target->on_destroy();
			instances.erase(loc);
		}
	}
	
	bool done;
	friend class GameInstance;
	vector<GameInstance*> instances;
};
