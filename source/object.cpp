#include "object.hpp"

void game_object::accelerate() {
	acceleration -= acceleration_rate / 2.0f;
	if (acceleration > acceleration_limit) {
		acceleration = acceleration_limit;
	}
	if (acceleration < 0.0f) {
		acceleration = 0.0f;
	}
	transform.position.x += std::cos(transform.rotation.z) * acceleration;
	transform.position.y -= std::sin(transform.rotation.z) * acceleration;
}
