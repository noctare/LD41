#include "world.hpp"
#include "assets.hpp"
#include "game.hpp"

#include <graphics.hpp>
#include <camera.hpp>
#include <platform.hpp>
#include <simplex_noise.hpp>

base_chunk::base_chunk() {
	transform.scale.xy = 1.0f;
}

void base_chunk::set_index(const ne::vector2i& index) {
	this->index = index;
	transform.position.xy = (index * ne::vector2i{ pixel_width, pixel_height }).to<float>();
	if (offset_to_grid) {
		transform.position.x += (float)index.x * 8.0f;
		transform.position.y += (float)index.y * 8.0f;
	}
}

void base_chunk::draw() {
	if (needs_rendering) {
		render();
	}
	if (!shape.exists()) {
		return;
	}
	ne::shader::set_transform(&transform);
	shape.bind();
	shape.draw();
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
			} else if (tile == TILE_SLIME) {
				tile_uv = { 9, 1 };
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
	render_tile(TILE_SLIME);
	shape.upload();
	needs_rendering = false;
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

void object_chunk::render() {
	if (bones.size() == 0) {
		return;
	}
	if (shape.exists()) {
		shape.destroy();
	}
	shape.create();
	for (auto& bone : bones) {
		ne::vector2f uv1 = { (float)bone.type / 3.0f, 0.0f };
		ne::vector2f uv2 = { (float)(bone.type + 1) / 3.0f, 1.0f };
		bone.transform.scale.xy = textures.bones.frame_size().to<float>();
		shape.append_quad(bone.transform.position.xy, bone.transform.scale.xy, uv1, uv2);
	}
	shape.upload();
	needs_rendering = false;
}

game_world::game_world() {
	ne::set_simplex_noise_seed(std::time(nullptr));
	generator.world = this;
	ne::vector2i index;
	for (int i = 0; i < total_chunks; i++) {
		auto& chunk = chunks[i];
		auto& object_chunk = object_chunks[i];
		chunk.world = this;
		chunk.set_index(index);
		object_chunk.world = this;
		object_chunk.set_index(index);
		if (index.x == 0 || index.x == chunks_per_row - 1 || index.y == 0 || index.y == chunks_per_column - 1) {
			generator.border(chunk.index);
		} else {
			generator.normal(chunk.index);
		}
		if (++index.x % chunks_per_row == 0) {
			++index.y;
			index.x = 0;
		}
	}
	player.transform.position.x = (float)(chunks_per_row * base_chunk::pixel_width) / 2.0f;
	player.transform.position.y = (float)(chunks_per_column * base_chunk::pixel_height) / 2.0f;
	while (!is_free_at(player.transform.position.xy)) {
		player.transform.position.x += 20.0f;
	}
}

void game_world::update_items(std::vector<item_object>& items, int type, int max_of) {
	if (items.size() < max_of) {
		tile_chunk* player_chunk = chunk_at_world_position(player.transform.position.xy);
		if (player_chunk) {
			int x = -1;
			int y = -1;
			do {
				x = ne::random_int(0, base_chunk::tiles_per_row - 1);
				y = ne::random_int(0, base_chunk::tiles_per_column - 1);
			} while (player_chunk->tiles[y * base_chunk::tiles_per_row + x] == TILE_WALL);
			items.push_back({});
			items.back().transform.position.xy = player_chunk->transform.position.xy;
			items.back().transform.position.x += (float)x * (float)base_chunk::tile_pixel_size;
			items.back().transform.position.y += (float)y * (float)base_chunk::tile_pixel_size;
		}
	}
	for (int i = 0; i < (int)items.size(); i++) {
		auto& item = items[i];
		item.update(this);
		if (item.transform.collides_with(player.transform)) {
			if (type == ITEM_PILL) {
				if (++player.hearts > 3) {
					player.hearts = 3;
				}
			} else if (type == ITEM_INJECTION) {
				player.rush_started.start();
			}
			items.erase(items.begin() + i);
			i--;
		} else if (item.transform.distance_to(player.transform) > 512.0f) {
			items.erase(items.begin() + i);
			i--;
		}
	}
}

void game_world::update() {
	player.update(this);
	for (int i = 0; i < (int)blood_enemies.size(); i++) {
		auto& blood = blood_enemies[i];
		blood.update(this);
		if (blood.transform.distance_to(player.transform) > 512.0f) {
			blood_enemies.erase(blood_enemies.begin() + i);
			i--;
		}
	}
	for (int i = 0; i < (int)pimple_enemies.size(); i++) {
		auto& pimple = pimple_enemies[i];
		pimple.update(this);
	}
	update_items(pills, ITEM_PILL, 5);
	update_items(injections, ITEM_INJECTION, 2);
	if (blood_enemies.size() < 10) {
		tile_chunk* player_chunk = chunk_at_world_position(player.transform.position.xy);
		if (player_chunk) {
			int x = -1;
			int y = -1;
			do {
				x = ne::random_int(0, base_chunk::tiles_per_row - 1);
				y = ne::random_int(0, base_chunk::tiles_per_column - 1);
			} while (player_chunk->tiles[y * base_chunk::tiles_per_row + x] == TILE_WALL);
			blood_enemies.push_back({});
			blood_enemies.back().transform.position.xy = player_chunk->transform.position.xy;
			blood_enemies.back().transform.position.x += (float)x * (float)base_chunk::tile_pixel_size;
			blood_enemies.back().transform.position.y += (float)y * (float)base_chunk::tile_pixel_size;
		}
	}
	for (int i = 0; i < (int)bullets.size(); i++) {
		auto& bullet = bullets[i];
		bullet.update(this);
		bool destroy_i = false;
		if (!bullet.by_player) {
			if (bullet.transform.collides_with(player.transform)) {
				player.hearts--;
				destroy_i = true;
				bullet.has_hit_wall = false; // just a quickfix to avoid bullets breaking wall
			}
		}
		if (bullet.has_hit_wall) {
			ne::vector2f position = bullet.transform.position.xy + bullet.transform.scale.xy / 2.0f;
			tile_chunk* chunk = chunk_at_world_position(position);
			if (chunk) {
				auto tile = chunk->tile_at_world_position(position);
				if (tile.first && *tile.first == TILE_WALL) {
					if (bullet.can_destroy_wall) {
						*tile.first = TILE_BG_TOP;
						chunk->needs_rendering = true;
						if (tile.second.x == 0) {
							tile_chunk* left = at(chunk->index.x - 1, chunk->index.y);
							if (left) {
								left->needs_rendering = true;
							}
						} else if (tile.second.x == base_chunk::tiles_per_row - 1) {
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
						} else if (tile.second.y == base_chunk::tiles_per_column - 1) {
							tile_chunk* down = at(chunk->index.x, chunk->index.y + 1);
							if (down) {
								down->needs_rendering = true;
							}
						}
						if (bullet.by_player) {
							player.score++;
						}
					}
					destroy_i = true;
				}
			}
		}
		if (destroy_i) {
			bullets.erase(bullets.begin() + i);
			i--;
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
	textures.pimple.bind();
	animated_quad().bind();
	for (auto& pimple : pimple_enemies) {
		pimple.draw();
	}
	for (auto& bullet : bullets) {
		bullet.draw();
	}
	still_quad().bind();
	player.draw();
	textures.blood.bind();
	for (auto& blood : blood_enemies) {
		blood.draw();
	}
	textures.bones.bind();
	for (auto& chunk : object_chunks) {
		if (view.collides_with(chunk.transform.position.xy, chunk_pixel_size)) {
			chunk.draw();
		}
	}
	still_quad().bind();
	textures.pill.bind();
	for (auto& pill : pills) {
		pill.draw();
	}
	textures.injection.bind();
	for (auto& injection : injections) {
		injection.draw();
	}
	// Draw cursor:
	ne::vector2i mouse = game->camera.mouse().to<int>();
	//mouse.x -= mouse.x % tile_chunk::tile_pixel_size;
	//mouse.y -= mouse.y % tile_chunk::tile_pixel_size;
	ne::transform3f cursor;
	cursor.position.xy = mouse.to<float>();
	cursor.scale.xy = textures.cursor.size.to<float>();
	ne::shader::set_transform(&cursor);
	textures.cursor.bind();
	still_quad().bind();
	still_quad().draw();
}

tile_chunk* game_world::at(int x, int y) {
	if (x < 0 || y < 0 || x >= chunks_per_row || y >= chunks_per_column) {
		return nullptr;
	}
	return &chunks[y * chunks_per_row + x];
}

tile_chunk* game_world::chunk_at_world_position(const ne::vector2f& position) {
	ne::vector2i chunk_index = position.to<int>();
	chunk_index.x /= base_chunk::pixel_width;
	chunk_index.y /= base_chunk::pixel_height;
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
	// Check bones
	object_chunk& object_chunk = object_chunks[chunk->index.y * chunks_per_row + chunk->index.x];
	for (auto& bone : object_chunk.bones) {
		ne::transform3f collision = bone.transform;
		if (bone.type == 0) {
			collision.position.y += 64.0f;
		} else if (bone.type == 1) {
			collision.position.y += 48.0f;
		} else if (bone.type == 2) {
			collision.position.y += 16.0f;
		}
		collision.position.x += (float)(object_chunk.index.x * base_chunk::pixel_width);
		collision.position.y += (float)(object_chunk.index.y * base_chunk::pixel_height);
		collision.position.x += 4.0f;
		collision.scale.x -= 8.0f;
		if (collision.collides_with(position)) {
			return false;
		}
	}

	// Free
	return true;
}

void world_generator::normal(const ne::vector2i& index) {
	int tile_x = index.x * base_chunk::tiles_per_row;
	int tile_y = index.y * base_chunk::tiles_per_column;
	size_t chunk_index = index.y * game_world::chunks_per_row + index.x;
	if (chunk_index >= world->total_chunks) {
		return;
	}
	tile_chunk& chunk = world->chunks[chunk_index];
	for (int i = 0; i < base_chunk::total_tiles; i++) {
		int x = i % base_chunk::tiles_per_row;
		int y = i / base_chunk::tiles_per_row;
		float noise1 = ne::octave_noise(4, 0.35f, 0.05f, tile_x + x, tile_y + y);
		int type = TILE_WALL;
		if (noise1 > 0.0f) {
			type = TILE_BG_TOP;
			float noise2 = ne::octave_noise(4, 0.6f, 0.05f, tile_x + x, tile_y + y);
			float noise3 = ne::octave_noise(4, 0.5f, 0.05f, -128000 + tile_x + x, -128000 + tile_y + y);
			if (noise2 > 0.4f) {
				type = TILE_BG_BOTTOM;
			}
			if (noise2 > 0.35f && noise3 > 0.35f) {
				type = TILE_WALL;
			}
		}
		chunk.tiles[i] = type;
	}
	object_chunk& object_chunk = world->object_chunks[chunk_index];
	for (int i = 0; i < base_chunk::total_tiles; i++) {
		int x = i % base_chunk::tiles_per_row;
		int y = i / base_chunk::tiles_per_row;
		if (y > 5 && x % 2 != 0 && chunk.tiles[i] == TILE_WALL && chunk.tiles[i + 1] == TILE_WALL && ne::random_chance(0.7f)) {
			int j = i - base_chunk::tiles_per_row;
			int k = j - base_chunk::tiles_per_row;
			int l = k - base_chunk::tiles_per_row;
			bool left = (chunk.tiles[j] != TILE_WALL && chunk.tiles[k] != TILE_WALL && chunk.tiles[l] != TILE_WALL);
			bool right = (chunk.tiles[j + 1] != TILE_WALL && chunk.tiles[k + 1] != TILE_WALL && chunk.tiles[l + 1] != TILE_WALL);
			if (left && right) {
				int type = 0;
				int m = l - base_chunk::tiles_per_row;
				type += ((chunk.tiles[m] != TILE_WALL && chunk.tiles[m + 1] && ne::random_chance(0.5)) ? 1 : 0);
				if (type == 1) {
					int n = m - base_chunk::tiles_per_row;
					type += ((chunk.tiles[n] != TILE_WALL && chunk.tiles[n + 1] && ne::random_chance(0.5)) ? 1 : 0);
				}
				bone_object bone;
				bone.transform.position.xy = {
					(float)x * (float)base_chunk::tile_pixel_size,
					(float)(y - 7) * (float)base_chunk::tile_pixel_size
				};
				bone.type = type;
				object_chunk.bones.push_back(bone);
				continue;
			}
		}
		if (chunk.tiles[i] != TILE_WALL) {
			if (ne::random_chance(0.003f)) {
				world->pimple_enemies.push_back({});
				world->pimple_enemies.back().transform.position.xy = chunk.transform.position.xy;
				world->pimple_enemies.back().transform.position.x += (float)x * (float)base_chunk::tile_pixel_size;
				world->pimple_enemies.back().transform.position.y += (float)y * (float)base_chunk::tile_pixel_size;
			}
		}
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
