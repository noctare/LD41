#pragma once

#include "player.hpp"

#include <graphics.hpp>
#include <engine.hpp>

class player_object;
class game_world;

class world_generator {
public:

	void generate(int local_x, int local_y, int world_x, int world_y);

	void move_up();
	void move_left();
	void move_down();
	void move_right();

	game_world* world = nullptr;
	int row = 0;
	int column = 0;
	int row_world = 0;
	int column_world = 0;

};

class tile_chunk {
public:

	struct tile {
		static const int pixel_size = 16;
		ne::vector2i uv_index;
	};

	static const int tiles_per_row = 48;
	static const int tiles_per_column = 48;
	static const int total_tiles = tiles_per_row * tiles_per_column;
	static const int pixel_width = tiles_per_row * tile::pixel_size;
	static const int pixel_height = tiles_per_column * tile::pixel_size;

	static const bool offset_to_grid = true;

	ne::transform3f transform;
	ne::texture surface;
	tile tiles[total_tiles];
	bool is_dirty = false;

	bool is_open = false;

	tile_chunk();
	~tile_chunk();

	void render_tiles();
	void render(const ne::vector2i& position, ne::texture* source, const ne::vector2i& offset, const ne::vector2i& size);
	void render(const ne::vector2i& position, const tile& tile);
	void render(const ne::vector2i& position, ne::texture* source);
	void draw();
	void set_index(const ne::vector2i& index);

};

class game_world {
public:

	static const int chunks_per_row = 6;
	static const int chunks_per_column = 6;
	static const int total_chunks = chunks_per_row * chunks_per_column;

	tile_chunk chunks[total_chunks];

	player_object player;

	game_world();

	void update();
	void draw(const ne::transform3f& view);

private:

	world_generator generator;

};

