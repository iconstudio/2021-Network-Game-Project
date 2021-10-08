#pragma once
#include "pch.h"
#include "Behavior.h"
#include "resource.h"

constexpr double GRAVITY = km_per_hr(200.0);
constexpr double PLAYER_WALK_VELOCITY = km_per_hr(16.0);
constexpr double PLAYER_JUMP_VELOCITY = km_per_hr(60.0);
constexpr double BULLET_VELOCITY = km_per_hr(56.0);
constexpr double BULLET_EXPIRE_PERIOD = 0.35;

constexpr double START_WAIT_PERIOD = 1.0;
constexpr double CLEAR_WAIT_PERIOD = 2.0;

class oPlayer : public GameInstance {
public:
	oPlayer(double x, double y, GameWorldMesh* newmesh);

	virtual void on_create();
	virtual void on_update(double frame_advance);

	virtual void thud();

	int imxs;
	bool jumped = false;
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

	virtual void on_update(double frame_advance);

	virtual void thud();
	virtual void ceil();
	virtual void side();

	double time;
};

class oEnemy : public GameInstance {
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

	int root_level = 0;
};

class roomStart : public GameScene {
public:
	roomStart();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

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

class roomGame : public GameScene {
public:
	roomGame();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	HDC cutout_surface;
	HBITMAP cutout_bitmap;
	BLENDFUNCTION fade_blender;
	HDC fade_surface;
	HBITMAP fade_bitmap;

	SPAWN_DIFFICULTY difficulty;
	bool complete;
	double start_time;
	double clear_time;

	double view_x, view_y;
};
