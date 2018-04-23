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
	ne::texture player[2];
	ne::texture blood;
	ne::texture bullet;
	ne::texture cursor;
	ne::texture gun[2];
	ne::texture sword;
	ne::texture pill;
	ne::texture injection;
	ne::texture heart;
	ne::texture flame_boost;
	ne::texture mace;
	ne::texture eye_boss;
	ne::texture neuron;
	ne::texture pimple;
	ne::texture queen_slime;
	ne::texture slime;
	ne::texture slime_drop;
	ne::texture spike;
	ne::texture tapeworm_head;
	ne::texture tapeworm_body;
	ne::texture worm;
	ne::texture virus;
	ne::texture zindo_blood;
	ne::texture artery;
	ne::texture laser;
	ne::texture blood_bullet;
	ne::texture shotgun[2];
	ne::texture shotgun_bullet;
	ne::texture flamethrower[2];
	ne::texture flame_bullet;
	ne::texture player_2_idle[2];
	ne::texture player_2_walk[2];
	ne::texture menu_bg;

	void initialize();

};

class font_assets : public ne::font_group {
public:

	ne::font game_over;
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
