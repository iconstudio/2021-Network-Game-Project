#pragma once
#include "pch.h"
#include "Behavior.h"
#include "resource.h"

enum TILES : char {
	NONE = '0'
	, PLAYER = '@'
	, BLOCK = '1'
	, BROKEN_BLOCK = '3'
	, BLOCK_END = '0'
	, PLATE = '-'
	, WOODBOX = '#'
	, WOODROOTBOX = '$'
	, GATE = 'X'
	, SPAWN = '&'
	, AIRSPAWN = '^'
};

constexpr int TILE_IMAX = (VIEW_W / 16);
constexpr int TILE_JMAX = (VIEW_H / 16) + 1;
constexpr double GRAVITY = km_per_hr(200.0);
constexpr double PLAYER_WALK_VELOCITY = km_per_hr(16.0);
constexpr double PLAYER_JUMP_VELOCITY = km_per_hr(60.0);
constexpr double BULLET_VELOCITY = km_per_hr(56.0);

constexpr double START_WAIT_PERIOD = 1.0;
constexpr double CLEAR_WAIT_PERIOD = 2.0;

class oGraviton : public GameInstance {
public:
	oGraviton(double x, double y, GameWorldMesh* newmesh);

	virtual void jump(double power);
};

class oPlayer : public oGraviton {
public:
	oPlayer(double x, double y, GameWorldMesh* newmesh);

	virtual void on_create();
	virtual void on_update(double frame_advance);

	int imxs;
	double attack_cooldown;
	const double attack_period = 0.8;
	int attack_count;
	const int attack_count_max = 3;
	double attack_combo_time;
	const double attack_combo_period = 0.07;
};

class oPlayer2 : public oPlayer {};

class oPlayerBullet : public GameInstance {
public:
	oPlayerBullet(double x, double y, GameWorldMesh* newmesh);

	virtual void thud();
	virtual void ceil();
	virtual void side();
};

class oEnemy : public oGraviton {
	oEnemy(double x, double y, GameWorldMesh* newmesh);

public:
	int hp, maxhp;
	bool dead;
};

class oDoor : public GameInstance {
public:
	oDoor(double x, double y);

	void close();

	bool closed;
};

class oWoodbox : public GameInstance {
public:
	oWoodbox(double x, double y, GameWorldMesh* newmesh);
};

class roomGameTemplate : public GameScene {
public:
	roomGameTemplate();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	virtual void finish();

	HDC cutout_surface;
	HBITMAP cutout_bitmap;
	BLENDFUNCTION fade_blender;
	HDC fade_surface;
	HBITMAP fade_bitmap;

	SPAWN_DIFFICULTY difficulty;
	bool complete;
	double start_time;
	double clear_time;
};
