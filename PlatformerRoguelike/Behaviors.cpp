#include "pch.h"
#include "Behavior.h"


GameInstance::GameInstance(double x, double y)
	: x(x), y(y), image_index(0), image_speed(0), box {0, 0, 1, 1}
	, image_xscale(1), image_yscale(1), image_angle(0), image_alpha(1) {}

GameInstance::~GameInstance() {
	if (sprite_index)
		sprite_index.reset();
}

void GameInstance::on_create() {}

void GameInstance::on_destroy() {}

void GameInstance::on_update(double frame_advance) {
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

GameScene::GameScene() : done(false), instances{} {}

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
			instance.reset();
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
