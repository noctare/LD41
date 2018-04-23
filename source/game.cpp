#include "game.hpp"
#include "assets.hpp"

#include <SDL/ttf/SDL_ttf.h>

#include <graphics.hpp>
#include <platform.hpp>

game_state::game_state() {
	camera.target_chase_aspect.y = 2.0f;
	camera.target_chase_speed = { 0.25f, 0.25f };
	camera.zoom = 3.0f;

	world.game = this;

	ne::hide_mouse();

	key = ne::listen([&](ne::keyboard_key_message key) {
		if (!key.is_pressed) {
			return;
		}
		if (key.key == KEY_Z) {
			if (camera.zoom > 1.0f) {
				camera.zoom = 1.0f;
			} else {
				camera.zoom = 3.0f;
			}
		} else if (key.key == KEY_R) {
			if (game_over) {
				game_over = false;
				ne::swap_state<game_state>();
			}
		}
	});

	score_label.font = &fonts.hud;
	game_over_label.font = &fonts.game_over;
	game_over_label.render("Game over!");
	press_r_label.font = &fonts.game_over;
	press_r_label.render("Press 'R' to reset");
}

game_state::~game_state() {
	ne::erase(&key);
}

void game_state::update() {
	camera.transform.scale.xy = ne::window_size().to<float>();
	ui_camera.transform.scale.xy = ne::window_size().to<float>();

	camera.target = &world.player.transform;
	camera.update();

	if (!game_over) {
		world.update();
	}

	if (world.player.hearts < 1) {
		game_over = true;
	}

	score_label.render(STRING("Score: " << world.player.score));
	score_label.transform.position.x = ui_camera.width() / 2.0f - score_label.transform.scale.width / 2.0f;
	score_label.transform.position.y = 16.0f;

	game_over_label.transform.position.x = ui_camera.width() / 2.0f - game_over_label.transform.scale.width / 2.0f;
	game_over_label.transform.position.y = ui_camera.height() / 2.0f - game_over_label.transform.scale.height / 2.0f - 32.0f;

	press_r_label.transform.position.x = ui_camera.width() / 2.0f - press_r_label.transform.scale.width / 2.0f;
	press_r_label.transform.position.y = ui_camera.height() / 2.0f - press_r_label.transform.scale.height / 2.0f + 32.0f;

	debug.set(&fonts.debug, STRING(
		"Delta " << ne::delta() <<
		"\nFPS: " << ne::current_fps()
	));
}

void game_state::draw() {
	ne::transform3f view;
	// World
	shaders.basic.bind();
	ne::shader::set_color(1.0f);
	camera.bind();
	view.position.xy = camera.xy();
	view.scale.xy = camera.size();
	world.draw(view);
	// UI
	shaders.basic.bind();
	ui_camera.bind();
	view.position.xy = ui_camera.xy();
	view.scale.xy = ui_camera.size();
	ne::shader::set_color(1.0f);
	still_quad().bind();
	debug.draw(view);
	// Score
	score_label.draw();
	// Hearts
	textures.heart.bind();
	ne::transform3f heart;
	heart.scale.xy = textures.heart.size.to<float>() * 6.0f;
	heart.position.x = ui_camera.width() / 2.0f - ((float)world.player.hearts * (heart.scale.width + 8.0f)) / 2.0f;
	heart.position.y = 96.0f;
	for (int i = 0; i < world.player.hearts; i++) {
		ne::shader::set_transform(&heart);
		still_quad().draw();
		heart.position.x += heart.scale.width + 8.0f;
	}
	// Game over?
	if (game_over) {
		game_over_label.draw();
		press_r_label.draw();
	}
}

ne::drawing_shape& still_quad() {
	static ne::drawing_shape shape;
	return shape;
}

ne::drawing_shape& animated_quad() {
	static ne::drawing_shape shape;
	return shape;
}
