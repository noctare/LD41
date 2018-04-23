#pragma once

#include "object.hpp"

#define GUN_DEAGLE   0
#define GUN_SHOTGUN  1
#define GUN_FLAME    2

#define PLAYER_GHOST 0
#define PLAYER_PINK  1

class player_object : public game_object {
public:

	int64 score = 0;
	int gun = GUN_DEAGLE;
	int type = PLAYER_GHOST;

	ne::timer rush_started;
	int64 rush_max_ms = 3000;

	player_object();
	
	void update(game_world* world) override;
	void draw() override;

	void shoot(game_world* world);

private:

	ne::timer last_shot;
	int64 shoot_interval_ms = 250;
	ne::sprite_animation animation;

};
