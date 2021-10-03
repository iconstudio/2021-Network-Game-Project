#pragma once
#include "pch.h"
#include "Behavior.h"
#include "resource.h"

enum TILES : char {
	NONE = '0'
	, PLAYER = '@'
	, BLOCK = '#'
	, PLATE = '-'
	, BROKEN_BLOCK = '3'
	, WOODBOX = 'b'
	, WOODROOTBOX = 'B'
	, GATE = 'X'
};

enum class StaticDir : int {
	RIGHT = 1
	, LEFT = -1
	, UP = -1
	, DOWN = 1
};

constexpr int TILE_IMAX = (VIEW_W / 16);
constexpr int TILE_JMAX = (VIEW_H / 16) + 1;
constexpr double GRAVITY = km_per_hr(200.0);
constexpr double PLAYER_JUMP_VELOCITY = km_per_hr(60.0);
constexpr double BULLET_VELOCITY = km_per_hr(50.0);

class oGraviton : public GameInstance {
public:
	oGraviton(double x, double y, GameWorldMesh* newmesh);

	virtual void on_create();
	virtual void on_update(double frame_advance);

	void set_worldmesh(GameWorldMesh* newworld);

	double raycast_lt(double distance);
	double raycast_rt(double distance);
	double raycast_up(double distance);
	double raycast_dw(double distance);

	virtual void jump(double power);

	double gravity;

	sceneGame* system;
	GameWorldMesh* worldmesh;
};

class oPlayer : public oGraviton {
public:
	oPlayer(double x, double y, GameWorldMesh* newmesh);

	virtual void on_create();
	virtual void on_update(double frame_advance);

	StaticDir imxs;
	double attack_cooldown;
	const double attack_period = 0.8;
	int attack_count;
	const int attack_count_max = 3;
	double attack_combo_time;
	const double attack_combo_period = 0.07;
};

class oSolid : public GameInstance {
public:
};

class oPlayerBullet : public oGraviton {
public:
	oPlayerBullet(double x, double y, GameWorldMesh* newmesh);
};

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
	GameWorldMesh(sceneGame* room);
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
	sceneGame* my_room;

	HDC map_surface;
	HBITMAP map_bitmap;
	BLENDFUNCTION map_blend;

	vector<GameMeshPiece*> build_instances;
	vector<GameMeshPiece*> build_doodads;
	vector<GameMeshPiece*> build_terrain;
	vector<GameMeshPiece*> build_backtile;
};

class sceneGame : public GameScene {
public:
	sceneGame();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	unsigned long score = 0;

	GameWorldMesh worldmesh;
};
