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
	load({ &player[0], "player1.png" });
	load({ &player[1], "player1.png", 1, TEXTURE_FLIP_X });
	load({ &blood, "bloodcell.png" });

	spawn_thread();
	finish();
}

void font_assets::initialize() {
	root("assets/fonts");
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
	
}
