#pragma once

#include <transform.hpp>

class game_world;

class game_object {
public:

	int id = 0;
	ne::transform3f transform;

	float acceleration = 0.1f;
	float speed = 0.0f;

	virtual ~game_object() = default;

	virtual void update(game_world* world);
	virtual void draw() = 0;

protected:

	void accelerate();

};
