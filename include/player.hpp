#pragma once

#include "object.hpp"

class player_object : public game_object {
public:

	int64 score = 0;
	int hearts = 3;

	ne::timer rush_started;
	int64 rush_max_ms = 3000;

	player_object();
	
	void update(game_world* world) override;
	void draw() override;

	void shoot(game_world* world);

private:

	ne::timer last_shot;

};
