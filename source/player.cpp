#include "player.hpp"
#include "game.hpp"
#include "assets.hpp"

#include <graphics.hpp>

player_object::player_object() {
	transform.scale.xy = textures.player[0].size.to<float>();
}

void player_object::update(game_world* world) {
	game_object::update(world);
	float move_speed = speed;
	bool w = (ne::is_key_down(KEY_W) || ne::is_key_down(KEY_UP));
	bool a = (ne::is_key_down(KEY_A) || ne::is_key_down(KEY_LEFT));
	bool s = (ne::is_key_down(KEY_S) || ne::is_key_down(KEY_DOWN));
	bool d = (ne::is_key_down(KEY_D) || ne::is_key_down(KEY_RIGHT));
	if (a != d) {
		if (a) {
			transform.position.x -= move_speed;
			if (!world->is_free_at(transform.position.xy + ne::vector2f{ 0.0f, 8.0f })) {
				transform.position.x += move_speed;
			}
			direction = DIRECTION_LEFT;
		} else if (d) {
			transform.position.x += move_speed;
			if (!world->is_free_at(transform.position.xy + ne::vector2f{ transform.scale.width, 8.0f })) {
				transform.position.x -= move_speed;
			}
			direction = DIRECTION_RIGHT;
		}
		move_speed /= 2.0f;
	}
	if (w != s) {
		if (w) {
			transform.position.y -= move_speed;
			if (!world->is_free_at(transform.position.xy + ne::vector2f{ 8.0f, 0.0f })) {
				transform.position.y += move_speed;
			}
		} else if (s) {
			transform.position.y += move_speed;
			if (!world->is_free_at(transform.position.xy + ne::vector2f{ 8.0f, transform.scale.height })) {
				transform.position.y -= move_speed;
			}
		}
	}
	if ((w != s) || (a != d)) {
		accelerate();
	}
}

void player_object::draw() {
	textures.player[direction].bind();
	ne::shader::set_transform(&transform);
	still_quad().bind();
	still_quad().draw();
}
