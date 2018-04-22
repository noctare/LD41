#include "world.hpp"
#include "assets.hpp"
#include "game.hpp"

#include <graphics.hpp>
#include <camera.hpp>
#include <platform.hpp>
#include <simplex_noise.hpp>

tile_chunk::tile_chunk() {
	transform.scale.xy = 1.0f;
}

uint32* tile_chunk::at(int x, int y) {
	if (x < 0 || y < 0 || x >= tiles_per_row || y >= tiles_per_column) {
		ne::vector2i offset;
		if (x < 0) {
			offset.x = -1;
		} else if (x >= tiles_per_row) {
			offset.x = 1;
		}
		if (y < 0) {
			offset.y = -1;
		} else if (y >= tiles_per_column) {
			offset.y = 1;
		}
		tile_chunk* next = world->at(index.x + offset.x, index.y + offset.y);
		if (!next) {
			return nullptr;
		}
		if (offset.x == 1) {
			x -= tiles_per_row;
		} else if (offset.x == -1) {
			x += tiles_per_row;
		}
		if (offset.y == 1) {
			y -= tiles_per_column;
		} else if (offset.y == -1) {
			y += tiles_per_column;
		}
		return next->at(x, y);
	}
	return &tiles[tiles_per_row * y + x];
}

void tile_chunk::render_tile(int type) {
	if (!shape.exists()) {
		shape.create();
	}
	for (int x = 0; x < tiles_per_row; x++) {
		for (int y = 0; y < tiles_per_column; y++) {
			uint32 tile = tiles[y * tiles_per_row + x];
			if (tile != type) {
				continue;
			}

			ne::vector2f position = {
				(float)(x * tile_pixel_size),
				(float)(y * tile_pixel_size)
			};

			ne::vector2f size = (float)tile_pixel_size;

			ne::vector2i tile_uv;
			if (tile == TILE_BG_BOTTOM) {
				tile_uv = { 7, 1 };
			} else if (tile == TILE_BG_TOP) {
				tile_uv = { 4, 1 };
			} else if (tile == TILE_WALL) {
				tile_uv = { 1, 1 };
			}

			ne::vector2i uv = tile_uv * tile_pixel_size;

			ne::vector2f uv1 = {
				(float)uv.x / (float)textures.tiles.size.width,
				(float)uv.y / (float)textures.tiles.size.height
			};

			float step_x = size.x / (float)textures.tiles.size.width;
			float step_y = size.y / (float)textures.tiles.size.height;

			if (tile != TILE_BG_BOTTOM) {

				uint32* up = at(x, y - 1);
				uint32* down = at(x, y + 1);
				uint32* left = at(x - 1, y);
				uint32* right = at(x + 1, y);

				if (up && *up != tile) {
					uv1.y -= step_y / 4.0f;
					step_y += step_y / 4.0f;
					position.y -= 4.0f; // 16 / 4
					size.y += 4.0f;
				}

				if (down && *down != tile) {
					step_y += step_y / 4.0f;
					size.y += 4.0f; // 16 / 4
				}

				if (left && *left != tile) {
					uv1.x -= step_x / 4.0f;
					step_x += step_x / 4.0f;
					position.x -= 4.0f; // 16 / 4
					size.x += 4.0f;
				}

				if (right && *right != tile) {
					step_x += step_x / 4.0f;
					size.x += 4.0f; // 16 / 4
				}

			}

			ne::vector2f uv2 = {
				uv1.x + step_x,
				uv1.y + step_y
			};

			shape.append_quad(position, size, uv1, uv2);
		}
	}
}

void tile_chunk::render() {
	render_tile(TILE_BG_BOTTOM);
	render_tile(TILE_BG_TOP);
	render_tile(TILE_WALL);
	shape.upload();
}

void tile_chunk::draw() {
	if (!shape.exists()) {
		return;
	}
	ne::shader::set_transform(&transform);
	shape.bind();
	shape.draw();
}

void tile_chunk::set_index(const ne::vector2i& index) {
	this->index = index;
	transform.position.xy = (index * ne::vector2i{ pixel_width, pixel_height }).to<float>();
	if (offset_to_grid) {
		transform.position.x += (float)index.x * 8.0f;
		transform.position.y += (float)index.y * 8.0f;
	}
}

game_world::game_world() {
	ne::set_simplex_noise_seed(std::time(nullptr));
	generator.world = this;
	ne::vector2i index;
	for (auto& i : chunks) {
		i.world = this;
		i.set_index(index);
		generator.generate(i.index);
		if (++index.x % chunks_per_row == 0) {
			++index.y;
			index.x = 0;
		}
	}
	for (auto& i : chunks) {
		i.render();
	}
	player.transform.position.x = (float)(chunks_per_row * tile_chunk::pixel_width) / 2.0f;
	player.transform.position.y = 256.0f;
}

void game_world::update() {
	player.update(this);
	/*
	int x = (int)player.transform.position.x / tile_chunk::pixel_width;
	int y = (int)player.transform.position.y / tile_chunk::pixel_height;
	int local_x = x % chunks_per_row;
	int local_y = y % chunks_per_column;
	while (local_x < 0) {
		local_x += chunks_per_row;
	}
	while (local_y < 0) {
		local_y += chunks_per_column;
	}
	int i = local_y * game_world::chunks_per_row + local_x;
	*/
}

void game_world::draw(const ne::transform3f& view) {
	textures.tiles.bind();
	ne::shader::set_color(1.0f);
	ne::vector2f chunk_pixel_size = { (float)tile_chunk::pixel_width, (float)tile_chunk::pixel_height };
	for (auto& chunk : chunks) {
		if (view.collides_with(chunk.transform.position.xy, chunk_pixel_size)) {
			chunk.draw();
		}
	}
	player.draw();
}

tile_chunk* game_world::at(int x, int y) {
	if (x < 0 || y < 0 || x >= chunks_per_row || y >= chunks_per_column) {
		return nullptr;
	}
	return &chunks[y * chunks_per_row + x];
}

void world_generator::generate(const ne::vector2i& index) {
	int tile_x = index.x * tile_chunk::tiles_per_row;
	int tile_y = index.y * tile_chunk::tiles_per_column;
	size_t chunk_index = index.y * game_world::chunks_per_row + index.x;
	if (chunk_index >= world->total_chunks) {
		return;
	}
	tile_chunk& chunk = world->chunks[chunk_index];
	for (int i = 0; i < tile_chunk::total_tiles; i++) {
		int x = i % tile_chunk::tiles_per_row;
		int y = i / tile_chunk::tiles_per_row;
		int type = TILE_WALL;
		//float noise = ne::octave_noise(3.0f, 0.5f, 0.006f, tile_x + x, tile_y + y);
		float noise = ne::octave_noise(1.0f, 0.9f, 0.02f, tile_x + x, tile_y + y);
		if (noise > 0.6f) {
			type = TILE_BG_BOTTOM;
		} else if (noise > 0.0f) {
			type = TILE_BG_TOP;
		}
		chunk.tiles[i] = type;
	}
}
