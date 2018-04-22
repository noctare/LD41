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
	if (shape.exists()) {
		shape.destroy();
	}
	shape.create();
	render_tile(TILE_BG_BOTTOM);
	render_tile(TILE_BG_TOP);
	render_tile(TILE_WALL);
	shape.upload();
	needs_rendering = false;
}

void tile_chunk::draw() {
	if (needs_rendering) {
		render();
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

std::pair<uint32*, ne::vector2i> tile_chunk::tile_at_world_position(const ne::vector2f& position) {
	ne::vector2i tile_index = position.to<int>();
	tile_index.x -= index.x * pixel_width;
	tile_index.y -= index.y * pixel_height;
	tile_index.x -= tile_index.x % tile_pixel_size;
	tile_index.y -= tile_index.y % tile_pixel_size;
	tile_index.x /= tile_pixel_size;
	tile_index.y /= tile_pixel_size;
	return { at(tile_index.x, tile_index.y), tile_index };
}

game_world::game_world() {
	ne::set_simplex_noise_seed(std::time(nullptr));
	generator.world = this;
	ne::vector2i index;
	for (auto& i : chunks) {
		i.world = this;
		i.set_index(index);
		if (index.x == 0 || index.x == chunks_per_row - 1 || index.y == 0 || index.y == chunks_per_column - 1) {
			generator.border(i.index);
		} else {
			generator.normal(i.index);
		}
		if (++index.x % chunks_per_row == 0) {
			++index.y;
			index.x = 0;
		}
	}
	for (auto& i : chunks) {
		i.render();
	}
	player.transform.position.x = (float)(chunks_per_row * tile_chunk::pixel_width) / 2.0f;
	player.transform.position.y = (float)(chunks_per_column * tile_chunk::pixel_height) / 2.0f;
	while (!is_free_at(player.transform.position.xy)) {
		player.transform.position.x += 20.0f;
	}
}

void game_world::update() {
	player.update(this);
	for (int i = 0; i < (int)bullets.size(); i++) {
		auto& bullet = bullets[i];
		bullet.update(this);
		if (bullet.has_hit_wall) {
			ne::vector2f position = bullet.transform.position.xy + bullet.transform.scale.xy / 2.0f;
			tile_chunk* chunk = chunk_at_world_position(position);
			if (chunk) {
				auto tile = chunk->tile_at_world_position(position);
				if (tile.first && *tile.first == TILE_WALL) {
					*tile.first = TILE_BG_TOP;
					chunk->needs_rendering = true;
					if (tile.second.x == 0) {
						tile_chunk* left = at(chunk->index.x - 1, chunk->index.y);
						if (left) {
							left->needs_rendering = true;
						}
					} else if (tile.second.x == tile_chunk::tiles_per_row - 1) {
						tile_chunk* right = at(chunk->index.x + 1, chunk->index.y);
						if (right) {
							right->needs_rendering = true;
						}
					}
					if (tile.second.y == 0) {
						tile_chunk* up = at(chunk->index.x, chunk->index.y - 1);
						if (up) {
							up->needs_rendering = true;
						}
					} else if (tile.second.y == tile_chunk::tiles_per_column - 1) {
						tile_chunk* down = at(chunk->index.x, chunk->index.y + 1);
						if (down) {
							down->needs_rendering = true;
						}
					}
					player.score++;
					bullets.erase(bullets.begin() + i);
					i--;
				}
			}
		}
	}
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
	for (auto& bullet : bullets) {
		bullet.draw();
	}
}

tile_chunk* game_world::at(int x, int y) {
	if (x < 0 || y < 0 || x >= chunks_per_row || y >= chunks_per_column) {
		return nullptr;
	}
	return &chunks[y * chunks_per_row + x];
}

tile_chunk* game_world::chunk_at_world_position(const ne::vector2f& position) {
	ne::vector2i chunk_index = position.to<int>();
	chunk_index.x /= tile_chunk::pixel_width;
	chunk_index.y /= tile_chunk::pixel_height;
	return at(chunk_index.x, chunk_index.y);
}

bool game_world::is_free_at(const ne::vector2f& position) {
	tile_chunk* chunk = chunk_at_world_position(position);
	if (!chunk) {
		return false;
	}
	auto tile = chunk->tile_at_world_position(position);
	if (!tile.first) {
		NE_ERROR("No tile at world position " << position);
		return false;
	}
	if (*tile.first == TILE_WALL) {
		return false;
	}
	return true;
}

void world_generator::normal(const ne::vector2i& index) {
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
		float noise1 = ne::octave_noise(3, 0.7f, 0.01f, tile_x + x, tile_y + y);
		int type = TILE_WALL;
		if (noise1 > 0.0f) {
			type = TILE_BG_TOP;

			float noise2 = ne::octave_noise(5, 0.8f, 0.005f, tile_x + x, tile_y + y);
			float noise3 = ne::octave_noise(5, 0.8f, 0.005f, -128000 + tile_x + x, -128000 + tile_y + y);
			if (noise2 > 0.4f) {
				type = TILE_BG_BOTTOM;
			}
			if (noise2 > 0.35f && noise3 > 0.35f) {
				type = TILE_WALL;
			}

		}
		chunk.tiles[i] = type;
	}
}

void world_generator::border(const ne::vector2i& index) {
	size_t chunk_index = index.y * game_world::chunks_per_row + index.x;
	if (chunk_index >= world->total_chunks) {
		return;
	}
	tile_chunk& chunk = world->chunks[chunk_index];
	for (int i = 0; i < tile_chunk::total_tiles; i++) {
		chunk.tiles[i] = TILE_WALL;
	}
}
