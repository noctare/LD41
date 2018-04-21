#include "game.hpp"
#include "assets.hpp"

#include <graphics.hpp>
#include <platform.hpp>

game_state::game_state() {
	camera.target_chase_aspect.y = 1.5f;
	camera.target_chase_speed = { 0.25f, 0.5f };
	camera.zoom = 0.5f;
}

game_state::~game_state() {
	
}

void game_state::update() {
	camera.transform.scale.xy = ne::window_size().to<float>();
	ui_camera.transform.scale.xy = ne::window_size().to<float>();

	camera.target = &world.player.transform;
	camera.update();

	world.update();

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
}

ne::drawing_shape& still_quad() {
	static ne::drawing_shape shape;
	return shape;
}

ne::drawing_shape& animated_quad() {
	static ne::drawing_shape shape;
	return shape;
}
