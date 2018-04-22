#include "object.hpp"
#include "world.hpp"
#include "assets.hpp"
#include "game.hpp"

void game_object::update(game_world* world) {
	speed -= acceleration * slowdown_rate;
	if (speed < 0.0f) {
		speed = 0.0f;
	}
	if (move_directions != MOVE_DIRECTIONS_8) {
		return;
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
	if (speed < max_speed) {
		speed += acceleration;
	}
	if (speed > max_speed) {
		speed = max_speed;
	}
}

void game_object::move_left(game_world* world, float speed) {
	transform.position.x -= speed;
	collision_a = false;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 0.0f, 8.0f })) {
		if (affected_by_collision) {
			transform.position.x += speed;
		}
		collision_a = true;
	}
	direction = DIRECTION_LEFT;
}

void game_object::move_right(game_world* world, float speed) {
	transform.position.x += speed;
	collision_d = false;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ transform.scale.width, 8.0f })) {
		if (affected_by_collision) {
			transform.position.x -= speed;
		}
		collision_d = true;
	}
	direction = DIRECTION_RIGHT;
}

void game_object::move_up(game_world* world, float speed) {
	transform.position.y -= speed;
	collision_w = false;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 8.0f, 0.0f })) {
		if (affected_by_collision) {
			transform.position.y += speed;
		}
		collision_w = true;
	}
}

void game_object::move_down(game_world* world, float speed) {
	transform.position.y += speed;
	collision_s = false;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 8.0f, transform.scale.height })) {
		if (affected_by_collision) {
			transform.position.y -= speed;
		}
		collision_s = true;
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

void game_object::turn_forward(game_world* world, float speed) {
	const float delta_y = std::sin(-transform.rotation.z) * speed / angle_turn_speed_divider;
	transform.position.y += delta_y;
	if (!world->is_free_at(transform.position.xy + transform.scale.xy / 2.0f)) {
		if (affected_by_collision) {
			transform.position.y -= delta_y;
		}
		if (delta_y < 0.0f) {
			collision_s = true;
		} else {
			collision_w = true;
		}
	}
}

void game_object::turn_side(game_world* world, float speed) {
	const float delta_x = std::cos(-transform.rotation.z) * speed / angle_turn_speed_divider;
	transform.position.x += delta_x;
	if (!world->is_free_at(transform.position.xy + transform.scale.xy / 2.0f)) {
		if (affected_by_collision) {
			transform.position.x -= delta_x;
		}
		if (delta_x > 0.0f) {
			collision_d = true;
		} else {
			collision_a = true;
		}
	}
}

void game_object::turn_left(game_world* world, float speed, bool forward) {
	if (forward) {
		transform.rotation.z += max_angle_speed;
	} else {
		transform.rotation.z += min_angle_speed;
		turn_forward(world, speed);
	}
	turn_side(world, speed);
}

void game_object::turn_right(game_world* world, float speed, bool forward) {
	if (forward) {
		transform.rotation.z -= max_angle_speed;
	} else {
		transform.rotation.z -= min_angle_speed;
		turn_forward(world, speed);
	}
	turn_side(world, speed);
}

void game_object::move_forward(game_world* world, float speed) {
	const float delta_x = std::cos(transform.rotation.z) * speed;
	const float delta_y = -std::sin(transform.rotation.z) * speed;
	transform.position.x += delta_x;
	transform.position.y += delta_y;
	if (!world->is_free_at(transform.position.xy + transform.scale.xy / 2.0f)) {
		if (affected_by_collision) {
			transform.position.x -= delta_x;
			transform.position.y -= delta_y;
		}
		if (delta_x > 0.0f) {
			collision_d = true;
		} else {
			collision_a = true;
		}
		if (delta_y > 0.0f) {
			collision_s = true;
		} else {
			collision_w = true;
		}
	}
}

bullet_object::bullet_object(const ne::transform3f& origin, float angle) {
	transform.scale.xy = textures.bullet.size.to<float>();
	transform.position.xy = origin.position.xy + origin.scale.xy / 2.0f - transform.scale.xy / 2.0f;
	transform.rotation.z = angle;
	affected_by_collision = false;
	move_directions = MOVE_DIRECTIONS_360;
}

void bullet_object::update(game_world* world) {
	game_object::update(world);
	move_forward(world, 6.0f);
	if (collision_w || collision_a || collision_s || collision_d) {
		has_hit_wall = true;
	}
}

void bullet_object::draw() {
	textures.bullet.bind();
	ne::shader::set_transform(&transform);
	still_quad().bind();
	still_quad().draw();
}

enemy_object::enemy_object() {
	transform.scale.xy = textures.blood.size.to<float>();
	move_directions = MOVE_DIRECTIONS_360;
	random_bounce = ne::random_float(0.0f, 10000.0f);
}

void enemy_object::update(game_world* world) {
	bounce = std::sin((float)ne::ticks() / 200000.0f + random_bounce) * 2.0f;
	game_object::update(world);
	collision_w = false;
	collision_a = false;
	collision_s = false;
	collision_d = false;
	turn_left(world, speed, true);
	move_forward(world, speed);
	accelerate();
}

void enemy_object::draw() {
	textures.blood.bind();
	ne::transform3f draw_transform = transform;
	draw_transform.position.y -= bounce;
	draw_transform.scale.x += bounce / 8.0f;
	draw_transform.scale.y += bounce / 8.0f;
	draw_transform.position.x -= bounce / 8.0f;
	draw_transform.position.y -= bounce / 8.0f;
	ne::shader::set_transform(&draw_transform);
	still_quad().bind();
	still_quad().draw();
}
