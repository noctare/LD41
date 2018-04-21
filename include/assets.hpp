#pragma once

#include <asset.hpp>
#include <audio.hpp>

void load_assets();
void destroy_assets();

class texture_assets : public ne::texture_group {
public:

	ne::texture blank;
	ne::texture button;
	ne::texture tiles;
	ne::texture player;

	void initialize();

};

class font_assets : public ne::font_group {
public:

	ne::font hud;
	ne::font button;
	ne::font debug;

	void initialize();

};

class shader_assets : public ne::shader_group {
public:

	ne::shader basic;
	ne::shader light;

	void initialize();

};

class audio_assets : public ne::music_group {
public:

	void initialize();

};

texture_assets& _textures();
#define textures _textures()

font_assets& _fonts();
#define fonts _fonts()

shader_assets& _shaders();
#define shaders _shaders()

audio_assets& _audio();
#define audio _audio()
