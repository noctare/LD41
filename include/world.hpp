#pragma once

#include "player.hpp"

#include <graphics.hpp>
#include <engine.hpp>

#define TILE_BG_BOTTOM  0
#define TILE_BG_TOP     1
#define TILE_WALL       2

class player_object;
class game_world;

class world_generator {
public:

	void generate(const ne::vector2i& index);

	game_world* world = nullptr;

};

class tile_chunk {
public:

	static const int tile_pixel_size = 16;
	static const int tiles_per_row = 64;
	static const int tiles_per_column = 64;
	static const int total_tiles = tiles_per_row * tiles_per_column;
	static const int pixel_width = tiles_per_row * tile_pixel_size;
	static const int pixel_height = tiles_per_column * tile_pixel_size;

	static const bool offset_to_grid = false;

	game_world* world = nullptr;
	ne::transform3f transform;
	uint32 tiles[total_tiles];
	ne::drawing_shape shape;
	ne::vector2i index;

	tile_chunk();

	uint32* at(int x, int y);
	void render();
	void draw();
	void set_index(const ne::vector2i& index);

};

class game_world {
public:

	static const int chunks_per_row = 4;
	static const int chunks_per_column = 4;
	static const int total_chunks = chunks_per_row * chunks_per_column;

	tile_chunk chunks[total_chunks];

	player_object player;

	game_world();

	void update();
	void draw(const ne::transform3f& view);

	tile_chunk* at(int x, int y);

	world_generator generator;

	int previous_chunk_x = 0;
	int previous_chunk_y = 0;

};

