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

constexpr int TILE_IMAX = (VIEW_W / 16);
constexpr int TILE_JMAX = (VIEW_H / 16) + 1;

class oGraviton : public GameInstance {

};

class oPlayer : public oGraviton {

};

class oSolid : public GameInstance {

};

class oPlayerBullet : public GameInstance {

};

class sceneGame : public GameScene {
public:
	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	unsigned long score = 0;

	char** build_doodad_above;
	char** build_player;
	char** build_enemy;
	char** build_doodad;
	char** build_terrain;

	HDC map_surface;
	HBITMAP map_bitmap;
	BLENDFUNCTION map_blend;

	char** layer_doodad_above;
	char** layer_player;
	char** layer_enemy;
	char** layer_playerbullet;
	char** layer_enemybullet;
};
