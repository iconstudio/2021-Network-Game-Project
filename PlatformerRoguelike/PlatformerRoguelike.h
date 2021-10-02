#pragma once
#include "pch.h"
#include "Behavior.h"
#include "resource.h"

constexpr double meter_to_pixel = 16.;
constexpr double hour_to_seconds = 3600.;

constexpr double kph_to_pps = (1000.0 * meter_to_pixel / hour_to_seconds);

constexpr double km_per_hr(const double velocity) {
	return velocity * kph_to_pps;
}

constexpr double GRAVITY = km_per_hr(200.0);
constexpr double PLAYER_JUMP_VELOCITY = km_per_hr(60.0);
constexpr int TILE_IMAX = (VIEW_W / 16);
constexpr int TILE_JMAX = (VIEW_H / 16) + 1;

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
	RIGHT = 0
	, LEFT = 180
	, UP = 90
	, DOWN = 270
};

class MeshPiece {
public:
	MeshPiece();
	MeshPiece(const char ch);
	explicit operator char() const;
	MeshPiece& operator=(const char ch);
	bool operator==(const char ch) const;

	char data;
};

class GameWorldMesh {
public:
	GameWorldMesh(GameScene* room);
	~GameWorldMesh();

	void load(const char* mapfile);
	void build();
	void clear();
	void reset();

	MeshPiece* get_terrain(int ix, int iy) const;
	MeshPiece* place_terrain(double cx, double cy);
	bool place_free(double cx, double cy);
	bool place_throughable(double cx, double cy);
	bool place_collider(double cx, double cy);

	void on_render(HDC canvas);

private:
	GameScene* my_room;

	HDC map_surface;
	HBITMAP map_bitmap;
	BLENDFUNCTION map_blend;
	MeshPiece** build_instances;
	MeshPiece** build_doodads;

	vector<MeshPiece*> build_terrain;
	MeshPiece** build_backtile;
};

class oGraviton : public GameInstance {
public:
	oGraviton(GameWorldMesh* newmesh, double x, double y);

	virtual void on_create();
	virtual void on_update(double frame_advance);

	void set_worldmesh(GameWorldMesh* newworld);
	double raycast_lt(double distance);
	double raycast_rt(double distance);
	double raycast_up(double distance);
	double raycast_dw(double distance);

	void jump(double power);

	double hspeed, vspeed;
	double gravity;

private:
	GameWorldMesh* worldmesh;
	sceneGame* system;
};

class oPlayer : public oGraviton {
public:
	oPlayer(GameWorldMesh* newmesh, double x, double y);

	virtual void on_create();
	virtual void on_update(double frame_advance);

};

class oSolid : public GameInstance {
public:
};

class oPlayerBullet : public GameInstance {
public:
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
