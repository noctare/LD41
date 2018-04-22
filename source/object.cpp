#include "object.hpp"

void game_object::update(game_world* world) {
	speed -= acceleration * 0.5f;
	if (speed < 0.0f) {
		speed = 0.0f;
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
