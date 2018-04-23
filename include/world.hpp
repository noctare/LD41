#pragma once

#include "player.hpp"

#include <graphics.hpp>
#include <engine.hpp>

#define TILE_BG_BOTTOM  0
#define TILE_BG_TOP     1
#define TILE_WALL       2
#define TILE_SLIME      3

class player_object;
class game_world;
class game_state;

class world_generator {
public:

	void normal(const ne::vector2i& index);
	void border(const ne::vector2i& index);

	game_world* world = nullptr;

};

class base_chunk {
public:

	static const int tile_pixel_size = 16;
	static const int tiles_per_row = 16;
	static const int tiles_per_column = 16;
	static const int total_tiles = tiles_per_row * tiles_per_column;
	static const int pixel_width = tiles_per_row * tile_pixel_size;
	static const int pixel_height = tiles_per_column * tile_pixel_size;

	static const bool offset_to_grid = false;

	game_world* world = nullptr;
	ne::transform3f transform;
	ne::vector2i index;
	bool needs_rendering = true;
	ne::drawing_shape shape;

	base_chunk();

	void set_index(const ne::vector2i& index);
	void draw();
	virtual void render() = 0;

};

class tile_chunk : public base_chunk {
public:

	uint32 tiles[total_tiles];

	uint32* at(int x, int y);
	void render_tile(int type);
	void render() override;
	std::pair<uint32*, ne::vector2i> tile_at_world_position(const ne::vector2f& position);

};

class bone_object {
public:
	ne::transform3f transform;
	int type = 0;
};

class object_chunk : public base_chunk {
public:

	std::vector<bone_object> bones;

	void render() override;

};

class game_world {
public:

	static const int chunks_per_row = 32;
	static const int chunks_per_column = 32;
	static const int total_chunks = chunks_per_row * chunks_per_column;

	game_state* game = nullptr;

	tile_chunk chunks[total_chunks];
	object_chunk object_chunks[total_chunks];

	player_object player;
	std::vector<enemy_blood_object> blood_enemies;
	std::vector<enemy_pimple_object> pimple_enemies;
	std::vector<bullet_object> bullets;
	std::vector<item_object> pills;
	std::vector<item_object> injections;

	game_world();

	void update_items(std::vector<item_object>& items, int type, int max_of);

	void update();
	void draw(const ne::transform3f& view);

	tile_chunk* at(int x, int y);
	tile_chunk* chunk_at_world_position(const ne::vector2f& position);

	bool is_free_at(const ne::vector2f& position);

	world_generator generator;

};

