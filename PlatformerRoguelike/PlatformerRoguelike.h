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
	RIGHT = 0
	, LEFT = 180
	, UP = 90
	, DOWN = 270
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

	double gravity;
	GameWorldMesh* worldmesh;

protected:
	sceneGame* system;
};

class oPlayer : public oGraviton {
public:
	oPlayer(GameWorldMesh* newmesh, double x, double y);

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
	oPlayerBullet(GameWorldMesh* newmesh, double x, double y);
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
