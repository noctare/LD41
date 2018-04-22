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

	virtual ~game_object() = default;

	virtual void update(game_world* world);
	virtual void draw() = 0;

	void move_left(game_world* world, float speed);
	void move_right(game_world* world, float speed);
	void move_up(game_world* world, float speed);
	void move_down(game_world* world, float speed);
	void move(game_world* world, bool up, bool left, bool down, bool right);

protected:

	float bounce = 0.0f;
	float acceleration = 0.1f;
	float speed = 0.0f;
	float max_speed = 2.0f;
	float slowdown_rate = 0.5f;

	bool w = false;
	bool a = false;
	bool s = false;
	bool d = false;

	bool prev_w = false;
	bool prev_a = false;
	bool prev_s = false;
	bool prev_d = false;

	bool collision_w = false;
	bool collision_a = false;
	bool collision_s = false;
	bool collision_d = false;

	bool affected_by_collision = true;

	void accelerate();

};

class bullet_object : public game_object {
public:

	bool has_hit_wall = false;

	bullet_object(const ne::transform3f& origin, bool w, bool a, bool s, bool d);

	void update(game_world* world);
	void draw();

private:

	bool init_w = false;
	bool init_a = false;
	bool init_s = false;
	bool init_d = false;

};
