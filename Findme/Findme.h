#pragma once
#include "resource.h"
#include "Framework.h"


class oPlayer : public GameInstance {
public:
	oPlayer(char** mesh);

	virtual void on_create();
	virtual void on_update(double frame_advance);
	virtual void on_render(HDC canvas);
};
