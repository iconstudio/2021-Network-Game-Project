#include "pch.h"
#include "Behavior.h"


GameInstance::GameInstance(double x, double y, GameWorldMesh* newmesh)
	: x(x), y(y), image_index(0), image_speed(0), box {0, 0, 1, 1}
	, image_xscale(1), image_yscale(1), image_angle(0), image_alpha(1)
	, hspeed(0.0), vspeed(0.0), gravity(0.0)
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
				if (0 < hspeed) { // right

				} else { // left

				}

				hspeed = 0.0;
			}
		}

		double cast_y;
		double distance_y = abs(vspeed) * frame_advance;
		if (vspeed < 0) { // upward
			cast_y = raycast_up(distance_y);
		} else {
			cast_y = raycast_dw(distance_y);
		}

		if (worldmesh->place_collider(x, bbox_bottom() + 1)) {
			vspeed += gravity * frame_advance;
		} else {
			vspeed = 0.0;
		}
		if (0 < cast_y)
			vspeed = 0.0;
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
		if (!worldmesh->place_free(bbox_left() - 1, bbox_top())
			|| !worldmesh->place_free(bbox_left() - 1, bbox_bottom())) {
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
		if (!worldmesh->place_free(bbox_right() + 1, bbox_top())
			|| !worldmesh->place_free(bbox_right() + 1, bbox_bottom())) {
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
		if (!worldmesh->place_free(bbox_left(), bbox_top() - 1)
			|| !worldmesh->place_free(bbox_right(), bbox_top() - 1)) {
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
		if (!worldmesh->place_free(lx, by)
			|| !worldmesh->place_free(rx, by)) {
			break;
		}

		bool self_pt = worldmesh->place_throughable(x, y);
		auto bot_check = worldmesh->place_terrain(x, by);
		auto bot_pt = worldmesh->place_throughable(lx, by) || worldmesh->place_throughable(rx, by);
		auto next_check = worldmesh->place_terrain(x, y + 16);
		auto next_pt = worldmesh->place_throughable(lx, y + 16) || worldmesh->place_throughable(rx, y + 16);

		if (!self_pt && bot_pt) {
			if (bot_check == next_check) {
				break;
			}
		} else if (self_pt && bot_pt) {
			if (bot_check == next_check) {
				break;
			}
		}
		y++;
		move_distance--;
	}

	return move_distance;
}

GameScene::GameScene() : done(false), instances{} { instances.clear(); }

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
		for (auto& instance : instances) {
			delete instance;
		}
	}
	instances.clear();
}

void GameScene::on_update(double frame_advance) {
	if (!instances.empty()) {
		for (auto& instance : instances)
			instance->on_update(frame_advance);
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
}
