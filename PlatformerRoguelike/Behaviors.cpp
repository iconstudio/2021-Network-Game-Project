#include "pch.h"
#include "Behavior.h"


GameInstance::GameInstance(double x, double y, GameWorldMesh* newmesh)
	: x(x), y(y), image_index(0.0), image_speed(0.0), box {0, 0, 1, 1}
	, image_xscale(1.0), image_yscale(1.0), image_angle(0.0), image_alpha(1.0)
	, hspeed(0.0), vspeed(0.0), gravity(0.0), in_air(false)
	, room(nullptr), worldmesh(newmesh) {}

GameInstance::~GameInstance() {
	if (sprite_index)
		sprite_index.reset();
	if (room)
		room->instance_kill(this);
}

void GameInstance::on_create() {}

void GameInstance::on_destroy() {}

void GameInstance::on_update(double frame_advance) {
	if (!worldmesh) {
		x += hspeed * frame_advance;
		y += vspeed * frame_advance;
		if (gravity != 0.0)
			vspeed += gravity * frame_advance;
	} else {
		if (0 != hspeed) {
			double cast_x, distance_x = abs(hspeed) * frame_advance;

			if (0 < hspeed) {
				cast_x = raycast_rt(distance_x);
			} else {
				cast_x = raycast_lt(distance_x);
			}

			if (0 < cast_x) {
				side();
			}
		}

		vspeed += gravity * frame_advance;

		double cast_y, distance_y = abs(vspeed) * frame_advance;
		if (vspeed < 0) { // upward
			cast_y = raycast_up(distance_y);
		} else {
			cast_y = raycast_dw(distance_y);
		}

		if (0 < cast_y) {
			if (0 < vspeed) { // ceil
				ceil();
			} else { // land
				in_air = false;
				thud();
			}
		} else {
			in_air = true;
		}

		double by = bbox_bottom() + 1;
		if (worldmesh->place_collider(bbox_left(), by)
			|| worldmesh->place_collider(bbox_right(), by))
			in_air = false;
	}

	if (sprite_index) {
		double animation_speed;
		auto image_number = sprite_index->number;

		if (1 < image_number && 0.0 != (animation_speed = image_speed * frame_advance)) {
			image_index += animation_speed;

			// clipping
			while (image_index < 0) image_index += image_number;
			while (image_number <= image_index) image_index -= image_number;
		}
	}
}

void GameInstance::on_render(HDC canvas) {
	if (sprite_index) {
		sprite_index->draw(canvas, x, y, image_index, image_angle, image_xscale, image_yscale, image_alpha);
	}
}

void GameInstance::set_sprite(shared_ptr<GameSprite>& sprite) {
	sprite_index = sprite;
	set_mask(sprite);

	image_index = 0.0;
}

void GameInstance::set_mask(shared_ptr<GameSprite>& sprite) {
	CopyRect(&box, &(sprite->bbox));
}

void GameInstance::set_worldmesh(GameWorldMesh* newworld) {
	worldmesh = newworld;
}

double GameInstance::bbox_left() const {
	return x + box.left;
}

double GameInstance::bbox_top() const {
	return y + box.top;
}

double GameInstance::bbox_right() const {
	return x + box.right;
}

double GameInstance::bbox_bottom() const {
	return y + box.bottom;
}

bool GameInstance::collide_with(GameInstance*& other) {
	return !(other->bbox_right() < bbox_left() || other->bbox_bottom() < bbox_top()
			|| bbox_right() < other->bbox_left() || bbox_bottom() < other->bbox_top());
}

double GameInstance::raycast_lt(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (worldmesh->place_solid(bbox_left() - 1, bbox_top())
			|| worldmesh->place_solid(bbox_left() - 1, bbox_bottom())) {
			break;
		}
		x--;
		move_distance--;
	}

	return move_distance;
}

double GameInstance::raycast_rt(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (worldmesh->place_solid(bbox_right() + 1, bbox_top())
			|| worldmesh->place_solid(bbox_right() + 1, bbox_bottom())) {
			break;
		}
		x++;
		move_distance--;
	}

	return move_distance;
}

double GameInstance::raycast_up(double distance) {
	double move_distance = floor(distance * 400) / 400;
	while (0 < move_distance) {
		if (worldmesh->place_solid(bbox_left(), bbox_top() - 1)
			|| worldmesh->place_solid(bbox_right(), bbox_top() - 1)) {
			break;
		}
		y--;
		move_distance--;
	}

	return move_distance;
}

double GameInstance::raycast_dw(double distance) {
	double move_distance = floor(distance * 400) / 400;
	double lx = bbox_left();
	double rx = bbox_right();

	while (0 < move_distance) {
		double by = bbox_bottom() + 1;
		if (worldmesh->place_solid(lx, by)
			|| worldmesh->place_solid(rx, by)) {
			break;
		}

		auto self_what = worldmesh->place_terrain(x, y);
		bool self_pt = worldmesh->place_plate(lx, y) || worldmesh->place_plate(rx, y);

		auto bot_what_l = worldmesh->place_terrain(lx, by);
		auto bot_what_r = worldmesh->place_terrain(rx, by);
		auto bot_pt = worldmesh->place_plate(lx, by) || worldmesh->place_plate(rx, by);

		auto next_what_l = worldmesh->place_terrain(lx, y + 16);
		auto next_what_r = worldmesh->place_terrain(rx, y + 16);
		auto next_pt = worldmesh->place_plate(lx, y + 16)
			|| worldmesh->place_plate(rx, y + 16);

		if (!self_pt) {
			if (bot_pt) {
				break;
			}
		} else if (bot_pt && next_pt) {
			if (bot_what_l && bot_what_r && next_what_l && next_what_r
				&& (bot_what_l == next_what_l || bot_what_r == next_what_r)) {
				break;
			} else {
				y--;
			}
		}
		y++;
		move_distance--;
	}

	return move_distance;
}

void GameInstance::thud() {
	y = ::floor(y);
	vspeed = 0.0;
}

void GameInstance::ceil() {
	vspeed = 0.0;
}

void GameInstance::side() {
	x = ::floor(x);

	hspeed = 0.0;
}

GameScene::GameScene()
	: done(false), instances(), worldmesh(this) {
	instances.clear();
	instances.reserve(100);
}

GameScene::~GameScene() {
	instances.clear();
}

void GameScene::on_create() {
	if (!instances.empty()) {
		for (auto& instance : instances)
			instance->on_create();
	}
}

void GameScene::on_destroy() {
	if (!instances.empty()) {
		instances.erase(instances.begin(), instances.end());
	}

	instances.clear();
}

void GameScene::on_update(double frame_advance) {
	if (!instances.empty()) {
		for (auto& instance : instances) {
			instance->on_update(frame_advance);
		}
	}
}

void GameScene::on_render(HDC canvas) {
	if (!instances.empty()) {
		for (auto& instance : instances)
			instance->on_render(canvas);
	}
}

void GameScene::reset() {
	done = false;
	on_destroy();
	on_create();
}
