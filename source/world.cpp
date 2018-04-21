#include "world.hpp"
#include "assets.hpp"
#include "game.hpp"

#include <graphics.hpp>
#include <camera.hpp>
#include <platform.hpp>
#include <simplex_noise.hpp>

tile_chunk::tile_chunk() {
	transform.scale.xy = { (float)pixel_width, (float)pixel_height };
	surface.create();
	surface.parameters.are_pixels_in_memory = true;
	surface.size = { pixel_width, pixel_height };
	surface.pixels = new uint32[surface.size.width * surface.size.height];
	memset(surface.pixels, 0xFFFFFFFF, sizeof(uint32) * surface.size.width * surface.size.height);
	surface.render();
}

tile_chunk::~tile_chunk() {
	surface.destroy();
}

void tile_chunk::render(const ne::vector2i& position, ne::texture* source, const ne::vector2i& offset, const ne::vector2i& size) {
	const size_t surface_size = surface.size.width * surface.size.height;
	for (int y = 0; y < size.y; y++) {
		for (int x = 0; x < size.x; x++) {
			const uint32 value = source->pixels[(offset.y + y) * source->size.width + offset.x + x];
			if (((value & 0xFF000000) >> 24) == 0x00000000) {
				continue;
			}
			const size_t index = (position.y + y) * surface.size.width + position.x + x;
			if (index >= surface_size) {
				continue;
			}
			surface.pixels[index] = value;
		}
	}
	is_dirty = true;
}

void tile_chunk::render(const ne::vector2i& position, const tile& tile) {
	render(position, &textures.tiles, tile.uv_index * tile::pixel_size, tile::pixel_size);
}

void tile_chunk::render(const ne::vector2i& position, ne::texture* source) {
	render(position, source, 0, source->size);
}

void tile_chunk::render_tiles() {
	for (int y = 0; y < tiles_per_row; y++) {
		for (int x = 0; x < tiles_per_column; x++) {
			render({ x * tile::pixel_size, y * tile::pixel_size }, tiles[y * tiles_per_row + x]);
		}
	}
}

void tile_chunk::draw() {
	surface.bind();
	if (is_dirty) {
		surface.refresh();
		is_dirty = false;
	}
	ne::shader::set_transform(&transform);
	if (is_open) {
		ne::shader::set_color(0.5f, 0.0f, 1.0f, 1.0f);
	} else {
		ne::shader::set_color(1.0f);
	}
	still_quad().draw();
}

void tile_chunk::set_index(const ne::vector2i& index) {
	transform.position.xy = index.to<float>() * transform.scale.xy;
	if (offset_to_grid) {
		transform.position.x += (float)index.x * 8.0f;
		transform.position.y += (float)index.y * 8.0f;
	}
}

game_world::game_world() {
	generator.world = this;
	ne::set_simplex_noise_seed(std::time(nullptr));
	for (int i = 0; i < game_world::chunks_per_row; i++) {
		generator.move_up();
	}
	player.transform.position.x = (float)(chunks_per_row * tile_chunk::pixel_width) / 2.0f;
	player.transform.position.y = -1000.0f;
}

void game_world::update() {
	player.update(this);
	static int prev = -1;
	if (prev != -1) {
		chunks[prev].is_open = false;
	}
	int x = (int)player.transform.position.x / tile_chunk::pixel_width;
	int y = (int)player.transform.position.y / tile_chunk::pixel_height;
	y--;
	y = -y;
	int i = y * game_world::chunks_per_row + x;
	if (i < total_chunks && i >= 0) {
		chunks[i].is_open = true;
		if (i != prev) {
			generator.move_up();
		}
		prev = i;
	} else {
		prev = -1;
	}
}

void game_world::draw(const ne::transform3f& view) {
	for (auto& chunk : chunks) {
		chunk.draw();
	}
	player.draw();
}

void world_generator::generate(int local_x, int local_y, int world_x, int world_y) {
	int tile_world_x = world_x * tile_chunk::tiles_per_row;
	int tile_world_y = world_y * tile_chunk::tiles_per_column;
	tile_chunk& chunk = world->chunks[local_y * game_world::chunks_per_row + local_x];
	for (int i = 0; i < tile_chunk::total_tiles; i++) {
		int x = i % tile_chunk::tiles_per_row;
		int y = i / tile_chunk::tiles_per_row;
		int type = 0;
		//float noise = ne::octave_noise(3.0f, 0.5f, 0.006f, tile_world_x + x, tile_world_y + y);
		float noise = ne::octave_noise(1.0f, 0.9f, 0.006f, tile_world_x + x, tile_world_y + y);
		if (noise > 0.6f) {
			type = 2;
		} else if (noise > 0.0f) {
			type = 1;
		}
		chunk.tiles[i].uv_index.x = type;
	}
	chunk.set_index({ world_x, world_y });
	chunk.render_tiles();
}

void world_generator::move_up() {
	for (int i = 0; i < game_world::chunks_per_row; i++) {
		generate(i, row, column_world + i, row_world);
	}
	if (++row >= game_world::chunks_per_column) {
		row = 0;
	}
	--row_world;
}

void world_generator::move_left() {
	for (int i = 0; i < game_world::chunks_per_column; i++) {
		generate(column, i, column_world, row_world + i);
	}
	if (++column >= game_world::chunks_per_row) {
		column = 0;
	}
	++column_world;
}

void world_generator::move_down() {
	for (int i = 0; i < game_world::chunks_per_row; i++) {
		generate(i, row, column_world + i, row_world);
	}
	if (--row < 0) {
		row = game_world::chunks_per_column - 1;
	}
	++row_world;
}

void world_generator::move_right() {
	for (int i = 0; i < game_world::chunks_per_column; i++) {
		generate(column, i, column_world, row_world + i);
	}
	if (--column < 0) {
		column = game_world::chunks_per_row - 1;
	}
	--column_world;
}
