#pragma once

#include "object.hpp"

#define DIRECTION_LEFT  0
#define DIRECTION_RIGHT 1

class player_object : public game_object {
public:

	int direction = 0;

	player_object();
	
	void update(game_world* world) override;
	void draw() override;

};
