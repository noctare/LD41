#include "object.hpp"
#include "world.hpp"

void game_object::update(game_world* world) {
	speed -= acceleration * 0.5f;
	if (speed < 0.0f) {
		speed = 0.0f;
	}
	if (w || a || s || d) {
		move(world, w, a, s, d);
		prev_w = w;
		prev_a = a;
		prev_s = s;
		prev_d = d;
		return;
	}
	if (speed > 0.0f) {
		move(world, prev_w, prev_a, prev_s, prev_d);
		speed -= acceleration;
		if (speed < 0.0f) {
			speed = 0.0f;
			prev_w = false;
			prev_a = false;
			prev_s = false;
			prev_d = false;
		}
	}
}

void game_object::accelerate() {
	if (speed < 2.0f) {
		speed += acceleration;
	}
	if (speed > 2.0f) {
		speed = 2.0f;
	}
}

void game_object::move_left(game_world* world, float speed) {
	transform.position.x -= speed;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 0.0f, 8.0f })) {
		transform.position.x += speed;
	}
	direction = DIRECTION_LEFT;
}

void game_object::move_right(game_world* world, float speed) {
	transform.position.x += speed;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ transform.scale.width, 8.0f })) {
		transform.position.x -= speed;
	}
	direction = DIRECTION_RIGHT;
}

void game_object::move_up(game_world* world, float speed) {
	transform.position.y -= speed;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 8.0f, 0.0f })) {
		transform.position.y += speed;
	}
}

void game_object::move_down(game_world* world, float speed) {
	transform.position.y += speed;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 8.0f, transform.scale.height })) {
		transform.position.y -= speed;
	}
}

void game_object::move(game_world* world, bool up, bool left, bool down, bool right) {
	float move_speed = speed;
	if (left != right) {
		if (left) {
			move_left(world, move_speed);
		} else if (right) {
			move_right(world, move_speed);
		}
		move_speed /= 2.0f;
	}
	if (up != down) {
		if (up) {
			move_up(world, move_speed);
		} else if (down) {
			move_down(world, move_speed);
		}
	}
	if ((up != down) || (left != right)) {
		accelerate();
	}
}
