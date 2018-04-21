#include "menu.hpp"
#include "assets.hpp"
#include "game.hpp"

menu_state::menu_state() {
	play.label.font = &fonts.button;
	play.label.render("Play");
	play.sprite = &textures.button;
	play.transform.scale.xy = textures.button.frame_size().to<float>();
	play.button_shape = &animated_quad();
	play.label_shape = &still_quad();
	play.click.listen([&] {
		ne::swap_state<game_state>();
	});
	//camera.target = &play.transform;
	//camera.target_chase_aspect.y = 2.0f;
	//camera.target_chase_speed = { 0.2f, 0.1f };
}

menu_state::~menu_state() {

}

void menu_state::update() {
	camera.transform.scale.xy = ne::window_size().to<float>();
	camera.update();
	play.transform.position.x = camera.x() + camera.width() / 2.0f - play.transform.scale.width / 2.0f;
	play.transform.position.y = camera.y() + camera.height() / 2.0f - play.transform.scale.height / 2.0f;
	play.update();
}

void menu_state::draw() {
	shaders.basic.bind();
	camera.bind();
	play.draw();
}
