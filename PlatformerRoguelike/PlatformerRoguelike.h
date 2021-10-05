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
constexpr double BULLET_VELOCITY = km_per_hr(56.0);

class oGraviton : public GameInstance {
public:
	oGraviton(double x, double y, GameWorldMesh* newmesh);

	virtual void jump(double power);

	roomStart* system;
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

class roomStart : public GameScene {
public:
	roomStart();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	unsigned long score = 0;

	GameWorldMesh worldmesh;
};
