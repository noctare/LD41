#include "player.hpp"
#include "game.hpp"
#include "assets.hpp"

#include <graphics.hpp>

player_object::player_object() {
	transform.scale.xy = textures.player.size.to<float>();
}

void player_object::update(game_world* world) {
	if (ne::is_key_down(KEY_A) || ne::is_key_down(KEY_LEFT)) {
		if (!ne::is_key_down(KEY_W)) {
			transform.rotation.z += min_angle_speed;
			transform.position.y += std::sin(-transform.rotation.z) * speed.y / angle_turn_speed_divider;
		} else {
			transform.rotation.z += max_angle_speed;
		}
		transform.position.x += std::cos(-transform.rotation.z) * speed.x / angle_turn_speed_divider;
	}
	if (ne::is_key_down(KEY_D) || ne::is_key_down(KEY_RIGHT)) {
		if (!ne::is_key_down(KEY_W)) {
			transform.rotation.z -= min_angle_speed;
			transform.position.y += std::sin(-transform.rotation.z) * speed.y / angle_turn_speed_divider;
		} else {
			transform.rotation.z -= max_angle_speed;
		}
		transform.position.x += std::cos(-transform.rotation.z) * speed.x / angle_turn_speed_divider;
	}
	if (ne::is_key_down(KEY_W) || ne::is_key_down(KEY_UP)) {
		transform.position.x += std::cos(transform.rotation.z) * speed.x;
		transform.position.y -= std::sin(transform.rotation.z) * speed.y;
		acceleration += acceleration_rate;
	}
	if (ne::is_key_down(KEY_S) || ne::is_key_down(KEY_DOWN)) {
		transform.rotation.z += min_angle_speed;
		transform.position.x -= std::cos(transform.rotation.z) * 0.01f;
		transform.position.y += std::sin(transform.rotation.z) * 1.0f;
	}
	accelerate();
}

void player_object::draw() {
	textures.player.bind();
	ne::shader::set_transform(&transform);
	still_quad().bind();
	still_quad().draw();
}
