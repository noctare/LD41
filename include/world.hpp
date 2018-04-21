#pragma once

#include "player.hpp"

#include <graphics.hpp>
#include <engine.hpp>

class player_object;

class tile_chunk {
public:

	struct tile {
		static const int pixel_size = 16;
		ne::vector2i uv_index;
		tile() {
			uv_index.x = ne::random_int(0, 1);
		}
	};

	static const int tiles_per_row = 32;
	static const int total_tiles = tiles_per_row * tiles_per_row;

	ne::transform3f transform;
	ne::texture surface;
	tile tiles[total_tiles];
	bool is_dirty = false;

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

	static const int chunks_per_row = 8;
	static const int total_chunks = chunks_per_row * chunks_per_row;

	tile_chunk chunks[total_chunks];

	player_object player;

	game_world();

	void update();
	void draw(const ne::transform3f& view);

};
