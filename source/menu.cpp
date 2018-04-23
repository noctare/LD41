#include "menu.hpp"
#include "assets.hpp"
#include "game.hpp"

menu_state::menu_state() {
	play1.label.font = &fonts.button;
	//play1.label.render("Play");
	play1.sprite = &textures.button;
	play1.transform.scale.xy = textures.button.frame_size().to<float>();
	play1.button_shape = &animated_quad();
	play1.label_shape = &still_quad();
	play1.click.listen([&] {
		ne::swap_state<game_state>(PLAYER_GHOST);
	});
	play2.label.font = &fonts.button;
	//play2.label.render("Play");
	play2.sprite = &textures.button;
	play2.transform.scale.xy = textures.button.frame_size().to<float>();
	play2.button_shape = &animated_quad();
	play2.label_shape = &still_quad();
	play2.click.listen([&] {
		ne::swap_state<game_state>(PLAYER_PINK);
	});
	select.font = &fonts.game_over;
	select.render("Select your fighter");
}

menu_state::~menu_state() {

}

void menu_state::update() {
	camera.transform.scale.xy = ne::window_size().to<float>();
	camera.update();
	play1.transform.position.x = camera.width() / 2.0f - play1.transform.scale.width / 2.0f - 96.0f;
	play1.transform.position.y = 128.0f;
	play1.update();
	play2.transform.position.x = camera.width() / 2.0f - play2.transform.scale.width / 2.0f + 96.0f;
	play2.transform.position.y = 128.0f;
	play2.update();
	select.transform.position.x = camera.width() / 2.0f - select.transform.scale.width / 2.0f;
	select.transform.position.y = 32.0f;
}

void menu_state::draw() {
	shaders.basic.bind();
	ne::shader::set_color(1.0f);
	camera.bind();
	ne::transform3f transform;
	transform.scale.xy = camera.size();
	ne::shader::set_transform(&transform);
	textures.menu_bg.bind();
	still_quad().bind();
	still_quad().draw();
	animated_quad().bind();
	play1.draw();
	play2.draw();
	ne::shader::set_color(1.0f);
	ne::transform3f t = play1.transform;
	t.position.xy = t.position.xy + t.scale.xy / 2.0f - textures.player[0].size.to<float>() * 2.0f;
	t.scale.xy = textures.player[0].size.to<float>() * 4.0f;
	textures.player[0].bind();
	ne::shader::set_transform(&t);
	still_quad().bind();
	still_quad().draw();
	t = play2.transform;
	t.position.xy = t.position.xy + t.scale.xy / 2.0f - textures.player_2.size.to<float>() * 2.0f;
	t.scale.xy = textures.player_2.size.to<float>() * 4.0f;
	textures.player_2.bind();
	ne::shader::set_transform(&t);
	still_quad().draw();
	
	t.scale.xy = textures.menu_title.size.to<float>() * 4.0f;
	t.position.x = camera.width() / 2.0f - t.scale.width / 2.0f;
	t.position.y = camera.height() - t.scale.height - 8.0f;
	ne::shader::set_transform(&t);
	textures.menu_title.bind();
	still_quad().draw();

	ne::shader::set_color(0.0f, 0.0f, 0.0f, 1.0f);
	select.draw();
}
