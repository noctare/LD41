#include "player.hpp"
#include "game.hpp"
#include "assets.hpp"

#include <graphics.hpp>
#include <math.hpp>

player_object::player_object() {
	transform.scale.xy = textures.player[0].size.to<float>();
	last_shot.start();
}

void player_object::update(game_world* world) {
	if (ne::is_key_down(KEY_SPACE) || ne::is_mouse_button_down(MOUSE_BUTTON_LEFT)) {
		shoot(world);
	}
	if (rush_started.has_started && rush_started.milliseconds() < rush_max_ms) {
		acceleration = 0.5f;
		max_speed = 4.0f;
		slowdown_rate = 0.2f;
	} else {
		acceleration = 0.1f;
		max_speed = 2.0f;
		slowdown_rate = 0.5f;
	}
	bounce = std::sin((float)ne::ticks() / 200000.0f) * 4.0f;
	w = (ne::is_key_down(KEY_W) || ne::is_key_down(KEY_UP));
	a = (ne::is_key_down(KEY_A) || ne::is_key_down(KEY_LEFT));
	s = (ne::is_key_down(KEY_S) || ne::is_key_down(KEY_DOWN));
	d = (ne::is_key_down(KEY_D) || ne::is_key_down(KEY_RIGHT));
	game_object::update(world);
	float angle = ne::rad_to_deg(angle_to_mouse);
	direction = (angle > 90.0f && angle < 270.0f) ? 1 : 0;
}

void player_object::draw() {
	ne::transform3f draw_transform = transform;
	draw_transform.position.y -= bounce;
	draw_transform.scale.x += bounce / 8.0f;
	draw_transform.scale.y += bounce / 8.0f;
	draw_transform.position.x -= bounce / 8.0f;
	draw_transform.position.y -= bounce / 8.0f;

	if (!is_immune() || (immunity_timer.milliseconds() / 200) % 2 == 0) {
		textures.player[direction].bind();
		ne::shader::set_transform(&draw_transform);
		still_quad().draw();
	}

	float angle = ne::rad_to_deg(angle_to_mouse);
	if (angle > 90.0f && angle < 270.0f) {
		angle -= 180.0f;
	}
	draw_transform.position.x += 12.0f * (direction == DIRECTION_RIGHT ? -1.0f : 1.3f);
	draw_transform.position.y += 2.0f;
	draw_transform.scale.xy = textures.gun[0].size.to<float>();
	draw_transform.rotation.z = ne::deg_to_rad(angle);
	ne::shader::set_transform(&draw_transform);
	textures.gun[direction].bind();
	still_quad().draw();
}

void player_object::shoot(game_world* world) {
	if (last_shot.milliseconds() < 250) {
		return;
	}

	ne::transform3f origin = transform;
	origin.position.x += 12.0f * (direction == DIRECTION_RIGHT ? -1.0f : 1.3f);
	origin.position.y += 2.0f;
	origin.position.y -= bounce;
	origin.position.x -= bounce / 8.0f;
	origin.position.y -= bounce / 8.0f;
	origin.scale.xy = textures.gun[0].size.to<float>();

	bullet_object bullet(origin, angle_to_mouse, true);
	bullet.by_player = true;
	world->bullets.push_back(bullet);
	last_shot.start();
}

bool player_object::is_immune() const {
	return immunity_timer.has_started && immunity_timer.milliseconds() < 2000;
}

void player_object::hurt(int damage) {
	if (is_immune()) {
		return;
	}
	hearts -= damage;
	immunity_timer.start();
}
