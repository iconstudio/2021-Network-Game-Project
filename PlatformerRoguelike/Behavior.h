#pragma once
#include "pch.h"
#include "Sprite.h"


enum TILES : char {
	NONE = '0'
	, PLAYER = '@'
	, BLOCK = '#'
	, BROKEN_BLOCK = '3'
	, BLOCK_END = '0'
	, PLATE = '-'
	, WOODBOX = 'b'
	, WOODROOTBOX = 'B'
	, GATE = 'X'
	, SPAWN = '&'
	, SPIKE_LT = '<'
	, SPIKE_RT = '>'
	, SPIKE_UP = '^'
	, SPIKE = '*'
	, AIRSPAWN = '^'
};

enum class SPAWN_DIFFICULTY { NO, VERYEASY, EASY, NORMAL, HARD };

enum class SPAWN_TYPES { NORMAL, AIR };

class GameMeshPiece {
public:
	GameMeshPiece(const char ch = '0');

	char data;
};

class GameSpawnPoint {
public:
	GameSpawnPoint(SPAWN_TYPES ctype, int cx, int cy);

	int x, y;
	double time_activate = 0.0;

	SPAWN_DIFFICULTY difficulty;
	SPAWN_TYPES type;
};

class GameWorldMesh {
public:
	GameWorldMesh(GameScene* room, int new_width, int new_height);
	~GameWorldMesh();

	void load(const char* mapfile);
	void set_tile(int ix, int iy, TILES tile);
	void make_tile(int ix, int iy);
	void build();
	void clear();
	void reset();

	GameMeshPiece* get_terrain(int ix, int iy) const;
	GameMeshPiece* place_terrain(double cx, double cy);
	bool place_solid(double cx, double cy);
	bool place_plate(double cx, double cy);
	bool place_collider(double cx, double cy);

	void on_render(HDC canvas);
	double playerx, playery;

	const int width, height, iwidth, iheight;

private:
	GameScene* my_room;

	HDC map_surface;
	HBITMAP map_bitmap;

	vector<GameMeshPiece*> build_instances;
	vector<GameMeshPiece*> build_doodads;
	vector<GameMeshPiece*> build_terrain;
	vector<GameMeshPiece*> build_backtile;
	vector<GameSpawnPoint*> spawns;
};

// ��ü ������Ʈ
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

	virtual void jump(double power);

	virtual void thud();
	virtual void ceil();
	virtual void side();

	shared_ptr<GameSprite> sprite_index; // ��������Ʈ
	RECT box; // �浹ü

	double image_index, image_speed; // �ִϸ��̼�
	double image_xscale, image_yscale, image_angle, image_alpha;
	double x, y; // ��ǥ

	double hspeed, vspeed;
	double gravity;
	bool in_air;

	GameScene* my_room; // �Ҽ� ���
	GameWorldMesh* worldmesh;
};

// ��� ������Ʈ
class GameScene {
public:
	GameScene(int new_width, int new_height);
	virtual ~GameScene();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	void reset();

	template<class _GObj, typename ...Ty>
	auto instance_create(Ty... args) {
		_GObj* lptr = new _GObj(args...);
		lptr->my_room = this;

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

	const int width, height;
	GameWorldMesh worldmesh;

	vector<GameInstance*> instances;
};
