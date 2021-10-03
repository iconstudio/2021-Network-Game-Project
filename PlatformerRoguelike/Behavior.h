#pragma once
#include "pch.h"
#include "Sprite.h"


// 개체 컴포넌트
class GameInstance {
public:
	GameInstance(GameInstance&) = default;
	GameInstance(GameInstance&&) = default;
	GameInstance(double x = 0.0, double y = 0.0);
	virtual ~GameInstance();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);

	void set_sprite(shared_ptr<GameSprite>& sprite);
	void set_mask(shared_ptr<GameSprite>& sprite);
	double bbox_left() const;
	double bbox_top() const;
	double bbox_right() const;
	double bbox_bottom() const;
	bool collide_with(GameInstance*& other);
	void set_worldmesh(GameWorldMesh* newworld);

	shared_ptr<GameSprite> sprite_index; // 스프라이트
	RECT box; // 충돌체

	double image_index, image_speed; // 애니메이션
	double image_xscale, image_yscale, image_angle, image_alpha;
	double x, y; // 좌표

	double hspeed, vspeed;

	GameScene* room; // 소속 장면
};

// 장면 컴포넌트
class GameScene {
public:
	GameScene();
	virtual ~GameScene();

	virtual void on_create();
	virtual void on_destroy();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);
	void reset();

	template<class _GObj, typename ...Ty>
	auto instance_create(Ty... args) {
		_GObj* lptr = new _GObj(args...);
		lptr->room = this;

		instances.push_back(lptr);

		return lptr;
	}

	template<class _GObj>
	void instance_kill(_GObj* target) {
		auto loc = find_if(instances.begin(), instances.end(), [target](const auto& lhs) {
			return (lhs == target);
		});

		if (loc != instances.end()) {
			target->on_destroy();
			instances.erase(loc);
		}
	}
	
	bool done;
	friend class GameInstance;
	vector<GameInstance*> instances;
};
