#include "world.hpp"
#include "assets.hpp"
#include "game.hpp"

#include <graphics.hpp>
#include <camera.hpp>
#include <platform.hpp>
#include <simplex_noise.hpp>

world_chunk::world_chunk() {
	transform.scale.xy = 1.0f;
}

void world_chunk::set_index(const ne::vector2i& index) {
	this->index = index;
	transform.position.xy = (index * ne::vector2i{ pixel_width, pixel_height }).to<float>();
	if (offset_to_grid) {
		transform.position.x += (float)index.x * 8.0f;
		transform.position.y += (float)index.y * 8.0f;
	}
}

void world_chunk::draw_tiles() {
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

void world_chunk::draw_slime() {
	ne::transform3f draw_transform;
	for (auto& slime : slime_tiles) {
		int x = slime.i % tiles_per_row;
		int y = slime.i / tiles_per_row;
		tile_data* below = at(x, y + 1);
		if (below) {
			if (below->type == TILE_SLIME || below->type == TILE_WALL) {
				continue;
			}
			if (below->extra >= TILE_EX_BONE_BASE_LEFT && below->extra <= TILE_EX_BONE_TOP_RIGHT) {
				continue;
			}
		}
		if (tiles[slime.i].extra >= TILE_EX_BONE_BASE_LEFT && tiles[slime.i].extra <= TILE_EX_BONE_TOP_RIGHT) {
			continue;
		}
		draw_transform.position.xy = transform.position.xy;
		draw_transform.position.x += (float)(x * tile_pixel_size) + 2.0f;
		draw_transform.position.y += (float)((y + 1) * tile_pixel_size) - 1.0f;
		draw_transform.scale.xy = textures.slime.frame_size().to<float>();
		ne::shader::set_transform(&draw_transform);
		slime.animation.draw();
	}
}

tile_data* world_chunk::at(int x, int y) {
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
		world_chunk* next = world->at(index.x + offset.x, index.y + offset.y);
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

void world_chunk::render_tile(int type) {
	for (int x = 0; x < tiles_per_row; x++) {
		for (int y = 0; y < tiles_per_column; y++) {
			tile_data tile = tiles[y * tiles_per_row + x];
			if (tile.type != type) {
				continue;
			}
			ne::vector2f position = {
				(float)(x * tile_pixel_size),
				(float)(y * tile_pixel_size)
			};
			ne::vector2f size = (float)tile_pixel_size;
			ne::vector2i tile_uv;
			if (tile.type == TILE_BG_BOTTOM) {
				tile_uv = { 7, 1 };
			} else if (tile.type == TILE_BG_TOP) {
				tile_uv = { 4, 1 };
			} else if (tile.type == TILE_WALL) {
				tile_uv = { 1, 1 };
			} else if (tile.type == TILE_SLIME) {
				tile_uv = { 10, 1 };
			}
			ne::vector2i uv = tile_uv * tile_pixel_size;
			ne::vector2f uv1 = {
				(float)uv.x / (float)textures.tiles.size.width,
				(float)uv.y / (float)textures.tiles.size.height
			};
			float step_x = size.x / (float)textures.tiles.size.width;
			float step_y = size.y / (float)textures.tiles.size.height;
			if (tile.type != TILE_BG_BOTTOM) {
				tile_data* up = at(x, y - 1);
				tile_data* down = at(x, y + 1);
				tile_data* left = at(x - 1, y);
				tile_data* right = at(x + 1, y);
				if (up && up->type != tile.type) {
					uv1.y -= step_y / 4.0f;
					step_y += step_y / 4.0f;
					position.y -= 4.0f; // 16 / 4
					size.y += 4.0f;
				}
				if (down && down->type != tile.type) {
					step_y += step_y / 4.0f;
					size.y += 4.0f; // 16 / 4
				}
				if (left && left->type != tile.type) {
					uv1.x -= step_x / 4.0f;
					step_x += step_x / 4.0f;
					position.x -= 4.0f; // 16 / 4
					size.x += 4.0f;
				}
				if (right && right->type != tile.type) {
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

void world_chunk::render_tile_ex(int from, int to) {
	ne::vector2f size = (float)tile_pixel_size;
	float step_x = size.x / (float)textures.tiles.size.width;
	float step_y = size.y / (float)textures.tiles.size.height;
	for (int x = 0; x < tiles_per_row; x++) {
		for (int y = 0; y < tiles_per_column; y++) {
			tile_data tile = tiles[y * tiles_per_row + x];
			if (tile.extra < from || tile.extra > to) {
				continue;
			}
			ne::vector2f position = {
				(float)(x * tile_pixel_size),
				(float)(y * tile_pixel_size)
			};
			ne::vector2i tile_uv;
			switch (tile.extra) {
			case TILE_EX_BONE_BASE_LEFT: tile_uv = { 12, 3 }; break;
			case TILE_EX_BONE_BASE_RIGHT: tile_uv = { 13, 3 }; break;
			case TILE_EX_BONE_TILE_LEFT: tile_uv = { 12, 2 }; break;
			case TILE_EX_BONE_TILE_RIGHT: tile_uv = { 13, 2 }; break;
			case TILE_EX_BONE_MID_LEFT: tile_uv = { 12, 1 }; break;
			case TILE_EX_BONE_MID_RIGHT: tile_uv = { 13, 1 }; break;
			case TILE_EX_BONE_TOP_LEFT: tile_uv = { 12, 0 }; break;
			case TILE_EX_BONE_TOP_RIGHT: tile_uv = { 13, 0 }; break;
			default: break;
			}
			ne::vector2i uv = tile_uv * tile_pixel_size;
			ne::vector2f uv1 = {
				(float)uv.x / (float)textures.tiles.size.width,
				(float)uv.y / (float)textures.tiles.size.height
			};
			ne::vector2f uv2 = {
				uv1.x + step_x,
				uv1.y + step_y
			};
			shape.append_quad(position, size, uv1, uv2);
		}
	}
}

void world_chunk::render() {
	if (shape.exists()) {
		shape.destroy();
	}
	shape.create();
	render_tile(TILE_BG_BOTTOM);
	render_tile(TILE_BG_TOP);
	render_tile_ex(TILE_EX_BONE_BASE_LEFT, TILE_EX_BONE_BASE_RIGHT);
	render_tile(TILE_SLIME);
	render_tile(TILE_WALL);
	render_tile_ex(TILE_EX_BONE_TILE_LEFT, TILE_EX_BONE_TOP_RIGHT);
	shape.upload();
	needs_rendering = false;
}

std::pair<tile_data*, ne::vector2i> world_chunk::tile_at_world_position(const ne::vector2f& position) {
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
	ne::set_simplex_noise_seed((uint32)std::time(nullptr));
	generator.world = this;
	ne::vector2i index;
	for (int i = 0; i < total_chunks; i++) {
		auto& chunk = chunks[i];
		chunk.world = this;
		chunk.set_index(index);
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
	player.transform.position.x = (float)(chunks_per_row * world_chunk::pixel_width) / 2.0f;
	player.transform.position.y = (float)(chunks_per_column * world_chunk::pixel_height) / 2.0f;
	while (!is_free_at(player.transform.position.xy)) {
		player.transform.position.x += 20.0f;
	}
}

void game_world::update_items(std::vector<item_object>& items, int type, int max_of) {
	if ((int)items.size() < max_of) {
		world_chunk* player_chunk = chunk_at_world_position(player.transform.position.xy);
		if (player_chunk) {
			int x = -1;
			int y = -1;
			do {
				x = ne::random_int(0, world_chunk::tiles_per_row - 1);
				y = ne::random_int(0, world_chunk::tiles_per_column - 1);
			} while (player_chunk->tiles[y * world_chunk::tiles_per_row + x].type == TILE_WALL);
			items.push_back({});
			items.back().transform.position.xy = player_chunk->transform.position.xy;
			items.back().transform.position.x += (float)x * (float)world_chunk::tile_pixel_size;
			items.back().transform.position.y += (float)y * (float)world_chunk::tile_pixel_size;
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
				player.score += 5;
			} else if (type == ITEM_INJECTION) {
				player.rush_started.start();
				player.score += 10;
			} else if (type == ITEM_SHOTGUN) {
				player.gun = GUN_SHOTGUN;
				player.score += 5;
			} else if (type == ITEM_FLAMETHROWER) {
				player.gun = GUN_FLAME;
				player.score += 5;
			}
			items.erase(items.begin() + i);
			i--;
		} else if (item.transform.distance_to(player.transform) > 512.0f) {
			items.erase(items.begin() + i);
			i--;
		}
	}
}

void game_world::spawn_objects(world_chunk& chunk) {
	if (blood_enemies.size() < 10) {
		int x = -1;
		int y = -1;
		do {
			x = ne::random_int(0, world_chunk::tiles_per_row - 1);
			y = ne::random_int(0, world_chunk::tiles_per_column - 1);
		} while (chunk.tiles[y * world_chunk::tiles_per_row + x].type == TILE_WALL);
		ne::vector2f position = chunk.transform.position.xy;
		position.x += (float)x * (float)world_chunk::tile_pixel_size;
		position.y += (float)y * (float)world_chunk::tile_pixel_size;
		if (player.transform.distance_to(position) > 128.0f) {
			blood_enemies.push_back({});
			blood_enemies.back().transform.position.xy = position;
		}
	}
	if (worm_enemies.size() < 5) {
		int x = -1;
		int y = -1;
		do {
			x = ne::random_int(0, world_chunk::tiles_per_row - 1);
			y = ne::random_int(0, world_chunk::tiles_per_column - 1);
		} while (chunk.tiles[y * world_chunk::tiles_per_row + x].type == TILE_WALL);
		ne::vector2f position = chunk.transform.position.xy;
		position.x += (float)x * (float)world_chunk::tile_pixel_size;
		position.y += (float)y * (float)world_chunk::tile_pixel_size;
		if (player.transform.distance_to(position) > 128.0f) {
			worm_enemies.push_back({});
			worm_enemies.back().transform.position.xy = position;
		}
	}
	if (slime_queens.size() < 2) {
		int x = -1;
		int y = -1;
		do {
			x = ne::random_int(0, world_chunk::tiles_per_row - 1);
			y = ne::random_int(0, world_chunk::tiles_per_column - 1);
		} while (chunk.tiles[y * world_chunk::tiles_per_row + x].type == TILE_WALL);
		ne::vector2f position = chunk.transform.position.xy;
		position.x += (float)x * (float)world_chunk::tile_pixel_size;
		position.y += (float)y * (float)world_chunk::tile_pixel_size;
		if (player.transform.distance_to(position) > 128.0f) {
			slime_queens.push_back({});
			slime_queens.back().transform.position.xy = position;
		}
	}
	if (viruses.size() < 2) {
		int x = -1;
		int y = -1;
		do {
			x = ne::random_int(0, world_chunk::tiles_per_row - 1);
			y = ne::random_int(0, world_chunk::tiles_per_column - 1);
		} while (chunk.tiles[y * world_chunk::tiles_per_row + x].type == TILE_WALL);
		ne::vector2f position = chunk.transform.position.xy;
		position.x += (float)x * (float)world_chunk::tile_pixel_size;
		position.y += (float)y * (float)world_chunk::tile_pixel_size;
		if (player.transform.distance_to(position) > 128.0f) {
			viruses.push_back({});
			viruses.back().transform.position.xy = position;
		}
	}
	return;
	if (eye_bosses.size() < 1) {
		int x = -1;
		int y = -1;
		do {
			x = ne::random_int(0, world_chunk::tiles_per_row - 1);
			y = ne::random_int(0, world_chunk::tiles_per_column - 1);
		} while (chunk.tiles[y * world_chunk::tiles_per_row + x].type == TILE_WALL);
		ne::vector2f position = chunk.transform.position.xy;
		position.x += (float)x * (float)world_chunk::tile_pixel_size;
		position.y += (float)y * (float)world_chunk::tile_pixel_size;
		if (player.transform.distance_to(position) > 128.0f) {
			eye_bosses.push_back({});
			eye_bosses.back().transform.position.xy = position;
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
	for (auto& pimple : pimple_enemies) {
		pimple.update(this);
	}
	for (int i = 0; i < (int)slime_queens.size(); i++) {
		auto& slime_queen = slime_queens[i];
		slime_queen.update(this);
		if (player.transform.collides_with(slime_queen.transform)) {
			player.hurt(1);
		}
		if (slime_queen.transform.distance_to(player.transform) > 512.0f) {
			slime_queens.erase(slime_queens.begin() + i);
			i--;
		}
	}
	for (int i = 0; i < (int)worm_enemies.size(); i++) {
		auto& worm = worm_enemies[i];
		worm.update(this);
		if (player.transform.collides_with(worm.transform)) {
			player.hurt(1);
		}
		if (worm.transform.distance_to(player.transform) > 512.0f) {
			worm_enemies.erase(worm_enemies.begin() + i);
			i--;
		}
	}
	for (int i = 0; i < (int)slime_enemies.size(); i++) {
		auto& slime = slime_enemies[i];
		slime.update(this);
		if (player.transform.collides_with(slime.transform)) {
			player.hurt(1);
		}
		if (slime.transform.distance_to(player.transform) > 512.0f) {
			slime_enemies.erase(slime_enemies.begin() + i);
			i--;
		}
	}
	for (auto& spike : spikes) {
		spike.update(this);
		if (player.transform.collides_with(spike.transform)) {
			player.hurt(1);
		}
	}
	for (auto& artery : arteries) {
		artery.update(this);
	}
	for (auto& zindo_blood : zindo_bloods) {
		zindo_blood.update(this);
	}
	for (int i = 0; i < (int)viruses.size(); i++) {
		auto& virus = viruses[i];
		virus.update(this);
		if (virus.transform.distance_to(player.transform) > 512.0f) {
			viruses.erase(viruses.begin() + i);
			i--;
		}
	}
	for (auto& neuron : neurons) {
		neuron.update(this);
		if (player.transform.collides_with(neuron.transform)) {
			player.hurt(1);
		}
	}
	for (int i = 0; i < (int)eye_bosses.size(); i++) {
		auto& eye_boss = eye_bosses[i];
		eye_boss.update(this);
		if (player.transform.collides_with(eye_boss.transform)) {
			player.hurt(1);
		}
		if (eye_boss.transform.distance_to(player.transform) > 512.0f) {
			eye_bosses.erase(eye_bosses.begin() + i);
			i--;
		}
	}
	update_items(pills, ITEM_PILL, 5);
	update_items(injections, ITEM_INJECTION, 2);
	update_items(shotguns, ITEM_SHOTGUN, 0);
	update_items(flamethrowers, ITEM_FLAMETHROWER, 0);

	world_chunk* player_chunk = chunk_at_world_position(player.transform.position.xy);
	if (player_chunk) {
		auto neighbours = neighbour_chunks(player_chunk->index.x, player_chunk->index.y);
		for (auto& neighbour : neighbours) {
			if (neighbour) {
				spawn_objects(*neighbour);
			}
		}
	}

	for (int i = 0; i < (int)bullets.size(); i++) {
		auto& bullet = bullets[i];
		bullet.update(this);
		bool destroy_i = false;
		if (!bullet.by_player) {
			if (bullet.transform.collides_with(player.transform)) {
				player.hurt(bullet.attack());
				destroy_i = true;
				bullet.has_hit_wall = false; // just a quickfix to avoid bullets breaking wall
			}
		} else {
			// WARNING: TERRIBLE CODE AHEAD. Deadline approaching...
			bool cont = false;
			for (int j = 0; j < (int)worm_enemies.size(); j++) {
				enemy_chaser_object& worm = worm_enemies[j];
				if (bullet.transform.collides_with(worm.transform)) {
					worm.hurt(bullet.attack());
					if (worm.hearts < 1) {
						player.score += 5;
						worm_enemies.erase(worm_enemies.begin() + j);
						audio.bullet[0].play(20);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)slime_enemies.size(); j++) {
				enemy_chaser_object& slime = slime_enemies[j];
				if (bullet.transform.collides_with(slime.transform)) {
					slime.hurt(bullet.attack());
					if (slime.hearts < 1) {
						player.score += 5;
						slime_enemies.erase(slime_enemies.begin() + j);
						audio.bullet[0].play(20);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)slime_queens.size(); j++) {
				enemy_slime_queen_object& slime_queen = slime_queens[j];
				if (bullet.transform.collides_with(slime_queen.transform)) {
					slime_queen.hurt(bullet.attack());
					if (slime_queen.hearts < 1) {
						player.score += 200;
						slime_queen.explode(this);
						shotguns.push_back({});
						shotguns.back().transform.position.xy = slime_queen.transform.position.xy + slime_queen.transform.scale.xy / 2.0f;
						slime_queens.erase(slime_queens.begin() + j);
						audio.bullet[0].play(20);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)viruses.size(); j++) {
				virus_object& virus = viruses[j];
				if (bullet.transform.collides_with(virus.transform)) {
					virus.hurt(bullet.attack());
					if (virus.hearts < 1) {
						player.score += 100;
						if (ne::random_chance(0.75f)) {
							flamethrowers.push_back({});
							flamethrowers.back().transform.position.xy = virus.transform.position.xy + virus.transform.scale.xy / 2.0f;
						}
						audio.bullet[0].play(20);
						viruses.erase(viruses.begin() + j);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)zindo_bloods.size(); j++) {
				zindo_blood_object& zindo_blood = zindo_bloods[j];
				if (bullet.transform.collides_with(zindo_blood.transform)) {
					zindo_blood.hurt(bullet.attack());
					if (zindo_blood.hearts < 1) {
						player.score += 50;
						if (ne::random_chance(0.2f)) {
							flamethrowers.push_back({});
							flamethrowers.back().transform.position.xy = zindo_blood.transform.position.xy + zindo_blood.transform.scale.xy / 2.0f;
						}
						audio.bullet[0].play(20);
						zindo_bloods.erase(zindo_bloods.begin() + j);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)arteries.size(); j++) {
				artery_object& artery = arteries[j];
				if (bullet.transform.collides_with(artery.transform)) {
					artery.hurt(bullet.attack());
					if (artery.hearts < 1) {
						player.score += 5;
						audio.bullet[0].play(20);
						arteries.erase(arteries.begin() + j);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)pimple_enemies.size(); j++) {
				enemy_pimple_object& pimple = pimple_enemies[j];
				if (bullet.transform.collides_with(pimple.transform)) {
					pimple.hurt(bullet.attack());
					if (pimple.hearts < 1) {
						player.score += 50;
						if (ne::random_chance(0.2f)) {
							shotguns.push_back({});
							shotguns.back().transform.position.xy = pimple.transform.position.xy + pimple.transform.scale.xy / 2.0f;
						}
						audio.bullet[0].play(20);
						pimple_enemies.erase(pimple_enemies.begin() + j);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)neurons.size(); j++) {
				neuron_object& neuron = neurons[j];
				if (bullet.transform.collides_with(neuron.transform)) {
					neuron.hurt(bullet.attack());
					if (neuron.hearts < 1) {
						player.score += 25;
						if (ne::random_chance(0.2f)) {
							shotguns.push_back({});
							shotguns.back().transform.position.xy = neuron.transform.position.xy;
						}
						audio.bullet[0].play(20);
						neurons.erase(neurons.begin() + j);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)blood_enemies.size(); j++) {
				enemy_blood_object& blood = blood_enemies[j];
				if (bullet.transform.collides_with(blood.transform)) {
					blood.hurt(bullet.attack());
					if (blood.hearts < 1) {
						player.score += 5;
						blood_enemies.erase(blood_enemies.begin() + j);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)spikes.size(); j++) {
				spike_object& spike = spikes[j];
				if (bullet.transform.collides_with(spike.transform)) {
					spike.hurt(bullet.attack());
					if (spike.hearts < 1) {
						player.score += 10;
						spikes.erase(spikes.begin() + j);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
			for (int j = 0; j < (int)eye_bosses.size(); j++) {
				eye_boss_object& eye_boss = eye_bosses[j];
				if (bullet.transform.collides_with(eye_boss.transform)) {
					eye_boss.hurt(bullet.attack());
					if (eye_boss.hearts < 1) {
						player.score += 1000;
						eye_bosses.erase(eye_bosses.begin() + j);
					}
					destroy_i = true;
					bullet.has_hit_wall = false;
					cont = true;
					bullets.erase(bullets.begin() + i);
					i--;
					break;
				}
			}
			if (cont) {
				continue;
			}
		}
		if (bullet.has_hit_wall) {
			ne::vector2f position = bullet.transform.position.xy + bullet.transform.scale.xy / 2.0f;
			world_chunk* chunk = chunk_at_world_position(position);
			if (chunk) {
				auto tile = chunk->tile_at_world_position(position);
				if (tile.first) {
					if (tile.first->type == TILE_WALL || tile.first->type == TILE_SLIME) {
						if (bullet.can_destroy_wall) {
							tile.first->health -= bullet.attack();
							if (tile.first->health < 1) {
								if (tile.first->type == TILE_SLIME) {
									for (int s = 0; s < (int)chunk->slime_tiles.size(); s++) {
										if (tile.first == &chunk->tiles[chunk->slime_tiles[s].i]) {
											chunk->slime_tiles.erase(chunk->slime_tiles.begin() + s);
											break;
										}
									}
								}
								tile.first->type = TILE_BG_TOP;
								chunk->needs_rendering = true;
								if (tile.second.x == 0) {
									world_chunk* left = at(chunk->index.x - 1, chunk->index.y);
									if (left) {
										left->needs_rendering = true;
									}
								} else if (tile.second.x == world_chunk::tiles_per_row - 1) {
									world_chunk* right = at(chunk->index.x + 1, chunk->index.y);
									if (right) {
										right->needs_rendering = true;
									}
								}
								if (tile.second.y == 0) {
									world_chunk* up = at(chunk->index.x, chunk->index.y - 1);
									if (up) {
										up->needs_rendering = true;
									}
								} else if (tile.second.y == world_chunk::tiles_per_column - 1) {
									world_chunk* down = at(chunk->index.x, chunk->index.y + 1);
									if (down) {
										down->needs_rendering = true;
									}
								}
								if (bullet.by_player) {
									player.score++;
								}
							}
						}
						destroy_i = true;
					}
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
	ne::vector2f chunk_pixel_size = { (float)world_chunk::pixel_width, (float)world_chunk::pixel_height };
	// todo: use camera position and turn into chunk indices
	for (auto& chunk : chunks) {
		if (view.collides_with(chunk.transform.position.xy, chunk_pixel_size)) {
			chunk.draw_tiles();
		}
	}
	animated_quad().bind();
	textures.slime_drop.bind();
	for (auto& chunk : chunks) {
		if (view.collides_with(chunk.transform.position.xy, chunk_pixel_size)) {
			chunk.draw_slime();
		}
	}
	textures.pimple.bind();
	for (auto& pimple : pimple_enemies) {
		if (!pimple.transform.collides_with(view)) {
			continue;
		}
		pimple.draw();
	}
	textures.worm.bind();
	for (auto& worm : worm_enemies) {
		worm.draw();
	}
	for (auto& virus : viruses) {
		virus.draw();
	}
	still_quad().bind();
	textures.bullet.bind();
	for (auto& bullet : bullets) {
		if (bullet.type != BULLET_NORMAL || !bullet.transform.collides_with(view)) {
			continue;
		}
		bullet.draw();
	}
	textures.laser.bind();
	for (auto& bullet : bullets) {
		if (bullet.type != BULLET_LASER || !bullet.transform.collides_with(view)) {
			continue;
		}
		bullet.draw();
	}
	textures.shotgun_bullet.bind();
	for (auto& bullet : bullets) {
		if (bullet.type != BULLET_SHOTGUN || !bullet.transform.collides_with(view)) {
			continue;
		}
		bullet.draw();
	}
	textures.flame_bullet.bind();
	for (auto& bullet : bullets) {
		if (bullet.type != BULLET_FLAME || !bullet.transform.collides_with(view)) {
			continue;
		}
		bullet.draw();
	}
	animated_quad().bind();
	textures.artery.bind();
	for (auto& artery : arteries) {
		if (!artery.transform.collides_with(view)) {
			continue;
		}
		artery.draw();
	}
	textures.blood_bullet.bind();
	for (auto& bullet : bullets) {
		if (bullet.type != BULLET_BLOOD || !bullet.transform.collides_with(view)) {
			continue;
		}
		bullet.draw();
	}
	still_quad().bind();
	player.draw();
	textures.blood.bind();
	for (auto& blood : blood_enemies) {
		blood.draw();
	}
	textures.queen_slime.bind();
	for (auto& slime_queen : slime_queens) {
		slime_queen.draw();
	}
	textures.slime.bind();
	for (auto& slime : slime_enemies) {
		slime.draw();
	}
	textures.pill.bind();
	for (auto& pill : pills) {
		pill.draw();
	}
	textures.injection.bind();
	for (auto& injection : injections) {
		injection.draw();
	}
	textures.shotgun[0].bind();
	for (auto& shotgun : shotguns) {
		shotgun.draw();
	}
	textures.flamethrower[0].bind();
	for (auto& flamethrower : flamethrowers) {
		flamethrower.draw();
	}
	textures.neuron.bind();
	for (auto& neuron : neurons) {
		if (!neuron.transform.collides_with(view)) {
			continue;
		}
		neuron.draw();
	}
	for (auto& eye_boss : eye_bosses) {
		eye_boss.draw();
	}
	textures.spike.bind();
	animated_quad().bind();
	for (auto& spike : spikes) {
		if (!spike.transform.collides_with(view)) {
			continue;
		}
		spike.draw();
	}
	textures.zindo_blood.bind();
	for (auto& zindo_blood : zindo_bloods) {
		if (!zindo_blood.transform.collides_with(view)) {
			continue;
		}
		zindo_blood.draw();
	}
	still_quad().bind();
	// Draw cursor:
	ne::vector2i mouse = game->camera.mouse().to<int>();
	//mouse.x -= mouse.x % tile_chunk::tile_pixel_size;
	//mouse.y -= mouse.y % tile_chunk::tile_pixel_size;
	ne::transform3f cursor;
	cursor.position.xy = mouse.to<float>();
	cursor.scale.xy = textures.cursor.size.to<float>();
	ne::shader::set_transform(&cursor);
	textures.cursor.bind();
	still_quad().draw();
}

world_chunk* game_world::at(int x, int y) {
	if (x < 0 || y < 0 || x >= chunks_per_row || y >= chunks_per_column) {
		return nullptr;
	}
	return &chunks[y * chunks_per_row + x];
}

std::vector<world_chunk*> game_world::neighbour_chunks(int x, int y) {
	std::vector<world_chunk*> neighbours;
	neighbours.push_back(at(x - 1, y - 1));
	neighbours.push_back(at(x, y - 1));
	neighbours.push_back(at(x + 1, y - 1));
	neighbours.push_back(at(x - 1, y));
	neighbours.push_back(at(x + 1, y));
	neighbours.push_back(at(x - 1, y + 1));
	neighbours.push_back(at(x, y + 1));
	neighbours.push_back(at(x + 1, y + 1));
	return neighbours;
}

world_chunk* game_world::chunk_at_world_position(const ne::vector2f& position) {
	ne::vector2i chunk_index = position.to<int>();
	chunk_index.x /= world_chunk::pixel_width;
	chunk_index.y /= world_chunk::pixel_height;
	return at(chunk_index.x, chunk_index.y);
}

bool game_world::is_free_at(const ne::vector2f& position) {
	world_chunk* chunk = chunk_at_world_position(position);
	if (!chunk) {
		return false;
	}
	auto tile = chunk->tile_at_world_position(position);
	if (!tile.first) {
		NE_ERROR("No tile at world position " << position);
		return false;
	}
	if (tile.first->type == TILE_WALL) {
		return false;
	}
	if (tile.first->type == TILE_SLIME) {
		return false;
	}
	if (tile.first->extra >= TILE_EX_BONE_BASE_LEFT && tile.first->extra <= TILE_EX_BONE_MID_RIGHT) {
		return false;
	}
	return true;
}

bool world_generator::add_bone(world_chunk& chunk, int i) {
	int x = i % world_chunk::tiles_per_row;
	int y = i / world_chunk::tiles_per_row;
	if (y > 5 && x % 2 != 0 && chunk.tiles[i].type == TILE_WALL && chunk.tiles[i + 1].type == TILE_WALL && ne::random_chance(0.4f)) {
		int j = i - world_chunk::tiles_per_row;
		int k = j - world_chunk::tiles_per_row;
		int l = k - world_chunk::tiles_per_row;
		bool left = (chunk.tiles[j].type != TILE_WALL && chunk.tiles[k].type != TILE_WALL && chunk.tiles[l].type != TILE_WALL);
		bool right = (chunk.tiles[j + 1].type != TILE_WALL && chunk.tiles[k + 1].type != TILE_WALL && chunk.tiles[l + 1].type != TILE_WALL);
		if (left && right) {
			int type = 0;
			int m = l - world_chunk::tiles_per_row;
			int n = m - world_chunk::tiles_per_row;
			type += ((chunk.tiles[m].type != TILE_WALL && chunk.tiles[m + 1].type && ne::random_chance(0.5)) ? 1 : 0);
			if (type == 1) {
				type += ((chunk.tiles[n].type != TILE_WALL && chunk.tiles[n + 1].type && ne::random_chance(0.5)) ? 1 : 0);
			}
			chunk.tiles[j].extra = TILE_EX_BONE_BASE_LEFT;
			chunk.tiles[j + 1].extra = TILE_EX_BONE_BASE_RIGHT;
			chunk.tiles[k].extra = TILE_EX_BONE_TILE_LEFT;
			chunk.tiles[k + 1].extra = TILE_EX_BONE_TILE_RIGHT;
			if (type == 0) {
				chunk.tiles[l].extra = TILE_EX_BONE_MID_LEFT;
				chunk.tiles[l + 1].extra = TILE_EX_BONE_MID_RIGHT;
				chunk.tiles[m].extra = TILE_EX_BONE_TOP_LEFT;
				chunk.tiles[m + 1].extra = TILE_EX_BONE_TOP_RIGHT;
			} else if (type == 1) {
				chunk.tiles[l].extra = TILE_EX_BONE_TILE_LEFT;
				chunk.tiles[l + 1].extra = TILE_EX_BONE_TILE_RIGHT;
				chunk.tiles[m].extra = TILE_EX_BONE_MID_LEFT;
				chunk.tiles[m + 1].extra = TILE_EX_BONE_MID_RIGHT;
				chunk.tiles[n].extra = TILE_EX_BONE_TOP_LEFT;
				chunk.tiles[n + 1].extra = TILE_EX_BONE_TOP_RIGHT;
			} else if (type == 2) {
				chunk.tiles[l].extra = TILE_EX_BONE_TILE_LEFT;
				chunk.tiles[l + 1].extra = TILE_EX_BONE_TILE_RIGHT;
				chunk.tiles[m].extra = TILE_EX_BONE_TILE_LEFT;
				chunk.tiles[m + 1].extra = TILE_EX_BONE_TILE_RIGHT;
				chunk.tiles[n].extra = TILE_EX_BONE_MID_LEFT;
				chunk.tiles[n + 1].extra = TILE_EX_BONE_MID_RIGHT;
				int o = n - world_chunk::tiles_per_row;
				chunk.tiles[o].extra = TILE_EX_BONE_TOP_LEFT;
				chunk.tiles[o + 1].extra = TILE_EX_BONE_TOP_RIGHT;
			}
			return true;
		}
	}
	return false;
}

bool world_generator::add_spike(world_chunk& chunk, int i) {
	int x = i % world_chunk::tiles_per_row;
	int y = i / world_chunk::tiles_per_row;
	if (y > 5 && x % 2 != 0 && chunk.tiles[i].type == TILE_WALL && chunk.tiles[i + 1].type == TILE_WALL && ne::random_chance(0.6f)) {
		int j = i - world_chunk::tiles_per_row;
		int k = j - world_chunk::tiles_per_row;
		int l = k - world_chunk::tiles_per_row;
		bool left = (chunk.tiles[j].type != TILE_WALL && chunk.tiles[k].type != TILE_WALL && chunk.tiles[l].type != TILE_WALL);
		bool right = (chunk.tiles[j + 1].type != TILE_WALL && chunk.tiles[k + 1].type != TILE_WALL && chunk.tiles[l + 1].type != TILE_WALL);
		if (!left || !right) {
			return false;
		}
		spike_object spike;
		spike.transform.position.xy = {
			(float)x * (float)world_chunk::tile_pixel_size,
			((float)y - 4.5f) * (float)world_chunk::tile_pixel_size
		};
		spike.transform.position.x += chunk.transform.position.x;
		spike.transform.position.y += chunk.transform.position.y;
		world->spikes.push_back(spike);
		return true;
	}
	return false;
}

void world_generator::normal(const ne::vector2i& index) {
	int tile_x = index.x * world_chunk::tiles_per_row;
	int tile_y = index.y * world_chunk::tiles_per_column;
	size_t chunk_index = index.y * game_world::chunks_per_row + index.x;
	if (chunk_index >= world->total_chunks) {
		return;
	}
	world_chunk& chunk = world->chunks[chunk_index];
	for (int i = 0; i < world_chunk::total_tiles; i++) {
		int x = i % world_chunk::tiles_per_row;
		int y = i / world_chunk::tiles_per_row;
		float noise1 = ne::octave_noise(4.0f, 0.35f, 0.05f, tile_x + x, tile_y + y);
		int type = TILE_WALL;
		if (noise1 > 0.0f) {
			type = TILE_BG_TOP;
			float noise2 = ne::octave_noise(4.0f, 0.6f, 0.05f, tile_x + x, tile_y + y);
			float noise3 = ne::octave_noise(4.0f, 0.5f, 0.05f, -128000 + tile_x + x, -128000 + tile_y + y);
			if (noise2 > 0.4f) {
				type = TILE_BG_BOTTOM;
			}
			if (noise2 > 0.35f && noise3 > 0.35f) {
				type = TILE_WALL;
			}
			if (noise2 > 0.31f) { 
				float noise4 = ne::octave_noise(5.0f, 0.7f, 0.05f, -64000 + tile_x + x, -64000 + tile_y + y);
				if (noise4 > 0.45f) {
					type = TILE_SLIME;
				}
			}
		}
		if (type == TILE_SLIME) {
			chunk.tiles[i].health = 4;
			chunk.slime_tiles.push_back({ i });
		}
		chunk.tiles[i].type = type;
	}
	for (int i = 0; i < world_chunk::total_tiles; i++) {
		int x = i % world_chunk::tiles_per_row;
		int y = i / world_chunk::tiles_per_row;
		bool added = add_bone(chunk, i);
		if (!added) {
			added = add_spike(chunk, i);
		}
		if (!added) {
			if (chunk.tiles[i].type != TILE_WALL && chunk.tiles[i].type != TILE_SLIME) {
				if (ne::random_chance(0.003f)) {
					world->pimple_enemies.push_back({});
					world->pimple_enemies.back().transform.position.xy = chunk.transform.position.xy;
					world->pimple_enemies.back().transform.position.x += (float)x * (float)world_chunk::tile_pixel_size;
					world->pimple_enemies.back().transform.position.y += (float)y * (float)world_chunk::tile_pixel_size;
				} else if (ne::random_chance(0.005f)) {
					world->arteries.push_back({});
					world->arteries.back().type = ne::random_int(4);
					world->arteries.back().transform.position.xy = chunk.transform.position.xy;
					world->arteries.back().transform.position.x += (float)x * (float)world_chunk::tile_pixel_size;
					world->arteries.back().transform.position.y += (float)(y - 4) * (float)world_chunk::tile_pixel_size;
				}
			} else if (x > 1 && y > 3 && chunk.tiles[i].type == TILE_WALL && chunk.tiles[i + 1].type == TILE_WALL) {
				int j = i - world_chunk::tiles_per_row;
				int k = j - world_chunk::tiles_per_row;
				int l = k - world_chunk::tiles_per_row;
				int jt1 = chunk.tiles[j].type;
				int jt2 = chunk.tiles[j + 1].type;
				int kt1 = chunk.tiles[k].type;
				int kt2 = chunk.tiles[k + 1].type;
				int lt1 = chunk.tiles[l].type;
				int lt2 = chunk.tiles[l + 1].type;
				if (jt1 != TILE_WALL && jt2 != TILE_WALL && kt1 != TILE_WALL && kt2 != TILE_WALL && lt1 != TILE_WALL && lt2 != TILE_WALL) {
					if (ne::random_chance(0.1f)) {
						world->zindo_bloods.push_back({});
						world->zindo_bloods.back().transform.position.xy = chunk.transform.position.xy;
						world->zindo_bloods.back().transform.position.x += (float)x * (float)world_chunk::tile_pixel_size;
						world->zindo_bloods.back().transform.position.y += (float)(y - 3) * (float)world_chunk::tile_pixel_size;
					} else if (ne::random_chance(0.2f)) {
						world->neurons.push_back({});
						world->neurons.back().transform.position.xy = chunk.transform.position.xy;
						world->neurons.back().transform.position.x += (float)x * (float)world_chunk::tile_pixel_size;
						world->neurons.back().transform.position.y += (float)(y - 4) * (float)world_chunk::tile_pixel_size;
					}
				}
			}
		}
	}
}

void world_generator::border(const ne::vector2i& index) {
	size_t chunk_index = index.y * game_world::chunks_per_row + index.x;
	if (chunk_index >= world->total_chunks) {
		return;
	}
	world_chunk& chunk = world->chunks[chunk_index];
	for (int i = 0; i < world_chunk::total_tiles; i++) {
		chunk.tiles[i].type = TILE_WALL;
		chunk.tiles[i].health = 127;
	}
}
