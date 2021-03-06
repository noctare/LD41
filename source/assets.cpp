#include "assets.hpp"

#include <engine.hpp>

struct asset_container {
	texture_assets _textures;
	font_assets _fonts;
	shader_assets _shaders;
	audio_assets _audio;
};

static asset_container* _assets = nullptr;

void load_assets() {
	if (_assets) {
		NE_WARNING("Already exists");
		return;
	}
	_assets = new asset_container();
	// Prepare information on how to load the assets.
	_assets->_textures.initialize();
	_assets->_fonts.initialize();
	_assets->_shaders.initialize();
	_assets->_audio.initialize();
	// Load all the assets in one go.
	_assets->_textures.load_all();
	_assets->_fonts.load_all();
	_assets->_shaders.load_all();
	_assets->_audio.load_all();
	// Process the loaded assets. Textures must be rendered, etc...
	_assets->_textures.process_some(1000);
	_assets->_fonts.process_some(1000);
	_assets->_shaders.process_some(1000);
	_assets->_audio.process_some(1000);
}

void destroy_assets() {
	if (!_assets) {
		return;
	}
	delete _assets;
	_assets = nullptr;
}

texture_assets& _textures() {
	return _assets->_textures;
}

font_assets& _fonts() {
	return _assets->_fonts;
}

shader_assets& _shaders() {
	return _assets->_shaders;
}

audio_assets& _audio() {
	return _assets->_audio;
}

void texture_assets::initialize() {
	root("assets/textures");

	load({ &blank, "blank.png" });
	load({ &button, "button.png", 3 });
	load({ &tiles, "tiles.png", 1, TEXTURE_PIXELS_IN_MEMORY });
	load({ &player[0], "player1.png", 1, TEXTURE_FLIP_X });
	load({ &player[1], "player1.png" });
	load({ &blood, "bloodcell.png" });
	load({ &bullet, "normalbullet.png" });
	load({ &cursor, "cursor.png" });
	load({ &gun[0], "deserteagle.png" });
	load({ &gun[1], "deserteagle.png", 1, TEXTURE_FLIP_X });
	load({ &sword, "oldsword.png" });
	load({ &pill, "pill.png" });
	load({ &injection, "injection.png" });
	load({ &heart, "heart_icon.png" });
	load({ &flame_boost, "flameboost.png", 4, TEXTURE_IS_ANIMATED });
	load({ &mace, "mace.png" });
	load({ &eye_boss, "eyeboss.png" });
	load({ &neuron, "neuron.png" });
	load({ &pimple, "pimple.png", 2 });
	load({ &queen_slime, "queenslime.png" });
	load({ &slime, "slime.png" });
	load({ &slime_drop, "slimedrop.png", 10, TEXTURE_IS_ANIMATED });
	load({ &spike, "spike.png", 8, TEXTURE_IS_ANIMATED });
	load({ &tapeworm_head, "tapeworm.png" });
	load({ &tapeworm_body, "tapewormbody.png" });
	load({ &worm, "worm.png", 2, TEXTURE_IS_ANIMATED });
	load({ &virus, "virus.png" });
	load({ &zindo_blood, "zindoblood.png", 10, TEXTURE_IS_ANIMATED });
	load({ &artery, "artery.png", 5 });
	load({ &laser, "laser.png" });
	load({ &blood_bullet, "bloodbullet.png", 5 });
	load({ &shotgun[0], "shotgun.png" });
	load({ &shotgun[1], "shotgun.png", 1, TEXTURE_FLIP_X });
	load({ &shotgun_bullet, "shotgunbullet.png" });
	load({ &flamethrower[0], "flamethrower.png" });
	load({ &flamethrower[1], "flamethrower.png", 1, TEXTURE_FLIP_X });
	load({ &flame_bullet, "flamebullet.png" });
	load({ &player_2_idle[0], "player2idle.png", 3, TEXTURE_IS_ANIMATED });
	load({ &player_2_idle[1], "player2idle.png", 3, TEXTURE_IS_ANIMATED | TEXTURE_FLIP_X });
	load({ &player_2_walk[0], "player2walk.png", 4, TEXTURE_IS_ANIMATED });
	load({ &player_2_walk[1], "player2walk.png", 4, TEXTURE_IS_ANIMATED | TEXTURE_FLIP_X });
	load({ &player_2, "player2.png" });
	load({ &menu_bg, "menuscreen.png" });
	load({ &menu_title, "textmenuscreen.png" });

	spawn_thread();
	finish();
}

void font_assets::initialize() {
	root("assets/fonts");
	load({ &game_over, "leo.ttf", 48, false });
	load({ &hud, "leo.ttf", 36, false });
	load({ &button, "leo.ttf", 20, false });
	load({ &debug, "leo.ttf", 16, false });
}

void shader_assets::initialize() {
	root("assets/shaders");
	load({ &basic, "basic" });
	load({ &light, "light" });
}

void audio_assets::initialize() {
	root("assets/music");

	root("assets/sounds");
	load({ &bg, "bg.ogg" });
	load({ &bullet[0], "bullet1.ogg" });
	load({ &bullet[1], "bullet2.ogg" });
	load({ &bullet[2], "bullet3.ogg" });
	load({ &beam, "beam.ogg" });
	load({ &slime, "slime.ogg" });
}
