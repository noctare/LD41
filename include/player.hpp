#pragma once

#include "object.hpp"

class player_object : public game_object {
public:

	player_object();
	
	void update(game_world* world) override;
	void draw() override;

};
