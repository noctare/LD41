#include "world.hpp"
#include "assets.hpp"
#include "game.hpp"

#include <graphics.hpp>
#include <camera.hpp>
#include <platform.hpp>

tile_chunk::tile_chunk() {
	transform.scale.xy = (float)(tile::pixel_size * tiles_per_row);
	surface.create();
	surface.parameters.are_pixels_in_memory = true;
	surface.size = tiles_per_row * tile::pixel_size;
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
		for (int x = 0; x < tiles_per_row; x++) {
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
	still_quad().draw();
}

void tile_chunk::set_index(const ne::vector2i& index) {
	transform.position.xy = index.to<float>() * transform.scale.xy;
}

game_world::game_world() {
	for (int x = 0; x < chunks_per_row; x++) {
		for (int y = 0; y < chunks_per_row; y++) {
			size_t i = y * chunks_per_row + x;
			chunks[i].set_index({ x, y });
			chunks[i].render_tiles();
		}
	}
}

void game_world::update() {
	player.update(this);
}

void game_world::draw(const ne::transform3f& view) {
	for (auto& chunk : chunks) {
		chunk.draw();
	}
	player.draw();
}
