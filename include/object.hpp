#pragma once

#include <transform.hpp>

class game_world;

#define DIRECTION_LEFT  0
#define DIRECTION_RIGHT 1

class game_object {
public:

	int id = 0;
	ne::transform3f transform;
	int direction = 0;

	float acceleration = 0.1f;
	float speed = 0.0f;

	float bounce = 0.0f;

	virtual ~game_object() = default;

	virtual void update(game_world* world);
	virtual void draw() = 0;

	void move_left(game_world* world, float speed);
	void move_right(game_world* world, float speed);
	void move_up(game_world* world, float speed);
	void move_down(game_world* world, float speed);
	void move(game_world* world, bool up, bool left, bool down, bool right);

protected:

	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;

	bool prev_w = false;
	bool prev_a = false;
	bool prev_s = false;
	bool prev_d = false;

	void accelerate();

};
