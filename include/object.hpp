#pragma once

#include <transform.hpp>

class game_world;

class game_object {
public:

	int id = 0;
	ne::transform3f transform;

	float acceleration = 0.0f;
	float acceleration_rate = 0.4f;
	float acceleration_limit = 9.0f;
	ne::vector2f speed = 6.0f;

	float min_angle_speed = 0.05f;
	float max_angle_speed = 0.08f;
	float angle_turn_speed_divider = 10.0f;

	virtual ~game_object() = default;

	virtual void update(game_world* world) = 0;
	virtual void draw() = 0;

protected:

	void accelerate();

};
