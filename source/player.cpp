#include "player.hpp"
#include "game.hpp"
#include "assets.hpp"

#include <graphics.hpp>

player_object::player_object() {
	transform.scale.xy = textures.player[0].size.to<float>();
	last_shot.start();
}

void player_object::update(game_world* world) {
	if (shoot_listener_id == -1) {
		shoot_listener_id = ne::listen([this, world](ne::keyboard_key_message key) {
			if (key.is_pressed && key.key == KEY_SPACE && last_shot.milliseconds() > 250) {
				if (w || a || s || d) {
					world->bullets.push_back({ transform, w, a, s, d });
				} else {
					world->bullets.push_back({ transform, prev_w, direction == DIRECTION_LEFT, prev_s, direction == DIRECTION_RIGHT });
				}
				last_shot.start();
			}
		});
	}
	bounce = std::sin((float)ne::ticks() / 200000.0f) * 4.0f;
	w = (ne::is_key_down(KEY_W) || ne::is_key_down(KEY_UP));
	a = (ne::is_key_down(KEY_A) || ne::is_key_down(KEY_LEFT));
	s = (ne::is_key_down(KEY_S) || ne::is_key_down(KEY_DOWN));
	d = (ne::is_key_down(KEY_D) || ne::is_key_down(KEY_RIGHT));
	game_object::update(world);
}

void player_object::draw() {
	textures.player[direction].bind();
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
