#pragma once

#include <transform.hpp>
#include <graphics.hpp>
#include <timer.hpp>

class game_world;

#define DIRECTION_LEFT  0
#define DIRECTION_RIGHT 1

#define MOVE_DIRECTIONS_8    0
#define MOVE_DIRECTIONS_360  1

#define ITEM_PILL      0
#define ITEM_INJECTION 1

class game_object {
public:

	int id = 0;
	ne::transform3f transform;
	int direction = 0;

	virtual ~game_object() = default;

	virtual void update(game_world* world);
	virtual void draw() = 0;

	// 8 directions:
	void move_left(game_world* world, float speed);
	void move_right(game_world* world, float speed);
	void move_up(game_world* world, float speed);
	void move_down(game_world* world, float speed);
	void move(game_world* world, bool up, bool left, bool down, bool right);

	// 360 directions:
	void turn_forward(game_world* world, float speed);
	void turn_side(game_world* world, float speed);
	void turn_left(game_world* world, float speed, bool forward);
	void turn_right(game_world* world, float speed, bool forward);
	void move_forward(game_world* world, float speed);

protected:

	int move_directions = MOVE_DIRECTIONS_8;

	float bounce = 0.0f;
	float random_bounce = 0.0f;
	
	float acceleration = 0.1f;
	float speed = 0.0f;
	float max_speed = 2.0f;
	float slowdown_rate = 0.5f;

	// 360:
	float min_angle_speed = 0.05f;
	float max_angle_speed = 0.08f;
	float angle_turn_speed_divider = 10.0f;
	float angle_to_mouse = 0.0f;

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
	bool can_destroy_wall = false;
	bool by_player = false;

	bullet_object(const ne::transform3f& origin, float angle, bool destroy_walls);

	void update(game_world* world);
	void draw();

};

class enemy_blood_object : public game_object {
public:

	enemy_blood_object();

	void update(game_world* world);
	void draw();

};

class enemy_pimple_object : public game_object {
public:

	bool is_up = false;

	enemy_pimple_object();

	void update(game_world* world);
	void draw();

private:

	ne::sprite_animation animation;
	ne::timer timer;
	bool can_shoot = false;
	int64 first_reset_ms = 0;

};

class enemy_worm_object : public game_object {
public:

	enemy_worm_object();

	void update(game_world* world);
	void draw();

private:

	ne::sprite_animation animation;
	ne::timer last_turn;
	int64 wait_ms = 0;

};

class item_object : public game_object {
public:
	
	item_object();

	void update(game_world* world);
	void draw();

};

class spike_object : public game_object {
public:

	spike_object();

	void update(game_world* world);
	void draw();

private:

	ne::sprite_animation animation;

};
