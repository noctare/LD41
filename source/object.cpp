#include "object.hpp"
#include "world.hpp"
#include "assets.hpp"
#include "game.hpp"

bool game_object::is_immune() const {
	return immunity_timer.has_started && immunity_timer.milliseconds() < immunity_lasts_ms;
}

void game_object::hurt(int damage) {
	if (is_immune()) {
		return;
	}
	hearts -= damage;
	immunity_timer.start();
}

bool game_object::should_draw() const {
	return !immunity_timer.has_started || immunity_timer.milliseconds() > 50;
}

void game_object::update(game_world* world) {
	angle_to_mouse = ne::deg_to_rad(transform.angle_to(world->game->camera.mouse()));
	speed -= acceleration * slowdown_rate;
	if (speed < 0.0f) {
		speed = 0.0f;
	}
	if (move_directions != MOVE_DIRECTIONS_8) {
		return;
	}
	if (w || a || s || d) {
		move(world, w, a, s, d);
		prev_w = w;
		prev_a = a;
		prev_s = s;
		prev_d = d;
		return;
	}
	if (speed > 0.0f) {
		move(world, prev_w, prev_a, prev_s, prev_d);
		speed -= acceleration;
		if (speed < 0.0f) {
			speed = 0.0f;
			prev_w = false;
			prev_a = false;
			prev_s = false;
			prev_d = false;
		}
	}
}

void game_object::accelerate() {
	if (speed < max_speed) {
		speed += acceleration;
	}
	if (speed > max_speed) {
		speed = max_speed;
	}
}

void game_object::move_left(game_world* world, float speed) {
	transform.position.x -= speed;
	collision_a = false;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 0.0f, 8.0f })) {
		if (affected_by_collision) {
			transform.position.x += speed;
		}
		collision_a = true;
	}
	direction = DIRECTION_LEFT;
}

void game_object::move_right(game_world* world, float speed) {
	transform.position.x += speed;
	collision_d = false;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ transform.scale.width, 8.0f })) {
		if (affected_by_collision) {
			transform.position.x -= speed;
		}
		collision_d = true;
	}
	direction = DIRECTION_RIGHT;
}

void game_object::move_up(game_world* world, float speed) {
	transform.position.y -= speed;
	collision_w = false;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 8.0f, 0.0f })) {
		if (affected_by_collision) {
			transform.position.y += speed;
		}
		collision_w = true;
	}
}

void game_object::move_down(game_world* world, float speed) {
	transform.position.y += speed;
	collision_s = false;
	if (!world->is_free_at(transform.position.xy + ne::vector2f{ 8.0f, transform.scale.height })) {
		if (affected_by_collision) {
			transform.position.y -= speed;
		}
		collision_s = true;
	}
}

void game_object::move(game_world* world, bool up, bool left, bool down, bool right) {
	float move_speed = speed;
	if (left != right) {
		if (left) {
			move_left(world, move_speed);
		} else if (right) {
			move_right(world, move_speed);
		}
		move_speed /= 2.0f;
	}
	if (up != down) {
		if (up) {
			move_up(world, move_speed);
		} else if (down) {
			move_down(world, move_speed);
		}
	}
	if ((up != down) || (left != right)) {
		accelerate();
	}
}

void game_object::turn_forward(game_world* world, float speed) {
	const float delta_y = std::sin(-transform.rotation.z) * speed / angle_turn_speed_divider;
	transform.position.y += delta_y;
	if (!world->is_free_at(transform.position.xy + transform.scale.xy / 2.0f)) {
		if (affected_by_collision) {
			transform.position.y -= delta_y;
		}
		if (delta_y < 0.0f) {
			collision_s = true;
		} else {
			collision_w = true;
		}
	}
}

void game_object::turn_side(game_world* world, float speed) {
	const float delta_x = std::cos(-transform.rotation.z) * speed / angle_turn_speed_divider;
	transform.position.x += delta_x;
	if (!world->is_free_at(transform.position.xy + transform.scale.xy / 2.0f)) {
		if (affected_by_collision) {
			transform.position.x -= delta_x;
		}
		if (delta_x > 0.0f) {
			collision_d = true;
		} else {
			collision_a = true;
		}
	}
}

void game_object::turn_left(game_world* world, float speed, bool forward) {
	if (forward) {
		transform.rotation.z += max_angle_speed;
	} else {
		transform.rotation.z += min_angle_speed;
		turn_forward(world, speed);
	}
	turn_side(world, speed);
}

void game_object::turn_right(game_world* world, float speed, bool forward) {
	if (forward) {
		transform.rotation.z -= max_angle_speed;
	} else {
		transform.rotation.z -= min_angle_speed;
		turn_forward(world, speed);
	}
	turn_side(world, speed);
}

void game_object::move_forward(game_world* world, float speed) {
	const float delta_x = std::cos(transform.rotation.z) * speed;
	const float delta_y = -std::sin(transform.rotation.z) * speed;
	transform.position.x += delta_x;
	transform.position.y += delta_y;
	if (!world->is_free_at(transform.position.xy + transform.scale.xy / 2.0f)) {
		if (affected_by_collision) {
			transform.position.x -= delta_x;
			transform.position.y -= delta_y;
		}
		if (delta_x > 0.0f) {
			collision_d = true;
		} else {
			collision_a = true;
		}
		if (delta_y > 0.0f) {
			collision_s = true;
		} else {
			collision_w = true;
		}
	}
}

bullet_object::bullet_object(const ne::transform3f& origin, float angle, bool destroy_walls, int type) : type(type) {
	can_destroy_wall = destroy_walls;
	if (type == BULLET_NORMAL) {
		transform.scale.xy = textures.bullet.size.to<float>();
	} else if (type == BULLET_LASER) {
		transform.scale.xy = textures.laser.size.to<float>();
	} else if (type == BULLET_BLOOD) {
		transform.scale.xy = textures.blood_bullet.frame_size().to<float>();
		animation.fps = 10.0f;
	} else if (type == BULLET_SHOTGUN) {
		transform.scale.xy = textures.shotgun_bullet.size.to<float>();
	} else if (type == BULLET_FLAME) {
		transform.scale.xy = textures.flame_bullet.size.to<float>();
	}
	transform.position.xy = origin.position.xy + origin.scale.xy / 2.0f - transform.scale.xy / 2.0f;
	transform.rotation.z = angle;
	affected_by_collision = false;
	move_directions = MOVE_DIRECTIONS_360;
	max_speed = 6.0f;
}

void bullet_object::update(game_world* world) {
	game_object::update(world);
	move_forward(world, max_speed);
	if (collision_w || collision_a || collision_s || collision_d) {
		has_hit_wall = true;
	}
}

void bullet_object::draw() {
	ne::shader::set_transform(&transform);
	if (type == BULLET_BLOOD) {
		animation.draw();
	} else {
		still_quad().draw();
	}
}

int bullet_object::attack() const {
	switch (type) {
	case BULLET_NORMAL: return 1;
	case BULLET_LASER: return 1;
	case BULLET_BLOOD: return 1;
	case BULLET_SHOTGUN: return 3;
	case BULLET_FLAME: return 1;
	default: return 1;
	}
}

enemy_blood_object::enemy_blood_object() {
	transform.scale.xy = textures.blood.size.to<float>();
	move_directions = MOVE_DIRECTIONS_360;
	random_bounce = ne::random_float(0.0f, 10000.0f);
	hearts = 1;
}

void enemy_blood_object::update(game_world* world) {
	bounce = std::sin((float)ne::ticks() / 200000.0f + random_bounce) * 2.0f;
	game_object::update(world);
	collision_w = false;
	collision_a = false;
	collision_s = false;
	collision_d = false;
	turn_left(world, speed, true);
	move_forward(world, speed);
	accelerate();
}

void enemy_blood_object::draw() {
	ne::transform3f draw_transform = transform;
	draw_transform.position.y -= bounce;
	draw_transform.scale.x += bounce / 8.0f;
	draw_transform.scale.y += bounce / 8.0f;
	draw_transform.position.x -= bounce / 8.0f;
	draw_transform.position.y -= bounce / 8.0f;
	ne::shader::set_transform(&draw_transform);
	still_quad().draw();
}

enemy_pimple_object::enemy_pimple_object() {
	transform.scale.xy = textures.pimple.frame_size().to<float>();
	timer.start();
	first_reset_ms = ne::random_int(2000);
	interval_ms = 1000 + ne::random_int(2000);
	hearts = 10;
}

void enemy_pimple_object::update(game_world* world) {
	if (first_reset_ms > 0 && timer.milliseconds() > first_reset_ms) {
		timer.start();
		first_reset_ms = -1;
		return;
	}
	if (is_up) {
		if (timer.milliseconds() > interval_ms * 2) {
			is_up = false;
		} else if (timer.milliseconds() > interval_ms && can_shoot) {
			world->bullets.push_back({ transform, ne::deg_to_rad(0.0f), false, BULLET_BLOOD });
			world->bullets.push_back({ transform, ne::deg_to_rad(45.0f), false, BULLET_BLOOD });
			world->bullets.push_back({ transform, ne::deg_to_rad(90.0f), false, BULLET_BLOOD });
			world->bullets.push_back({ transform, ne::deg_to_rad(135.0f), false, BULLET_BLOOD });
			world->bullets.push_back({ transform, ne::deg_to_rad(180.0f), false, BULLET_BLOOD });
			world->bullets.push_back({ transform, ne::deg_to_rad(225.0f), false, BULLET_BLOOD });
			world->bullets.push_back({ transform, ne::deg_to_rad(270.0f), false, BULLET_BLOOD });
			world->bullets.push_back({ transform, ne::deg_to_rad(315.0f), false, BULLET_BLOOD });
			can_shoot = false;
		}
	} else {
		if (timer.milliseconds() > interval_ms * 4) {
			is_up = true;
			can_shoot = true;
			timer.start();
		}
	}
}

void enemy_pimple_object::draw() {
	if (!should_draw()) {
		return;
	}
	ne::shader::set_transform(&transform);
	animation.frame = (is_up ? 0 : 1);
	animation.draw(false);
}

enemy_chaser_object::enemy_chaser_object() {
	last_turn.start();
	acceleration = 0.1f;
	max_speed = 1.0f;
	hearts = 5;
}

void enemy_chaser_object::update(game_world* world) {
	if (last_turn.milliseconds() > ne::random_int(1000) + wait_ms) {
		wait_ms = 0;
		float angle_to_player = world->player.transform.angle_to(transform);
		w = false;
		a = false;
		s = false;
		d = false;
		if (angle_to_player > 45.0f && angle_to_player < 135.0f) {
			d = true;
		} else if (angle_to_player > 225.0f && angle_to_player < 315.0f) {
			a = true;
		}
		if (angle_to_player > 135.0f && angle_to_player < 225.0f) {
			s = true;
		} else if (angle_to_player < 45.0f || angle_to_player > 315.0f) {
			w = true;
		}
		last_turn.start();
	}
	max_speed = max_speed_normal;
	if (hold.w > 0) {
		hold.w--;
		w = true;
		max_speed = max_speed_fast;
	}
	if (hold.s > 0) {
		hold.s--;
		s = true;
		max_speed = max_speed_fast;
	}
	if (hold.a > 0) {
		hold.a--;
		a = true;
		max_speed = max_speed_fast;
	}
	if (hold.d > 0) {
		hold.d--;
		d = true;
		max_speed = max_speed_fast;
	}
	game_object::update(world);
	if (collision_w) {
		w = false;
		s = true;
		wait_ms = 2000;
	} else if (collision_s) {
		w = true;
		s = false;
		wait_ms = 2000;
	}
	if (collision_a) {
		d = true;
		a = false;
		wait_ms = 2000;
	} else if (collision_d) {
		d = false;
		a = true;
		wait_ms = 2000;
	}
}

void enemy_chaser_object::draw() {
	if (!should_draw()) {
		return;
	}
	transform.scale.xy = ne::texture::bound()->frame_size().to<float>();
	ne::transform3f draw_transform = transform;
	if (direction == DIRECTION_RIGHT) {
		draw_transform.position.x += transform.scale.width;
		draw_transform.scale.width = -transform.scale.width;
	}
	ne::shader::set_transform(&draw_transform);
	animation.draw();
}

enemy_slime_queen_object::enemy_slime_queen_object() {
	hearts = 50;
	transform.scale.xy = textures.queen_slime.frame_size().to<float>();
	last_slime_drop.start();
}

void enemy_slime_queen_object::update(game_world* world) {
	bounce = std::sin((float)ne::ticks() / 300000.0f + random_bounce) * 2.0f;
	if (last_slime_drop.milliseconds() > 3000) {
		enemy_chaser_object slime;
		slime.max_speed_normal = 1.0f;
		slime.transform.position = transform.position;
		slime.transform.position.x += transform.scale.width / 2.0f - 4.0f;
		slime.transform.position.y += transform.scale.height - 4.0f;
		world->slime_enemies.push_back(slime);
		audio.slime.play(15);
		last_slime_drop.start();
	}
}

void enemy_slime_queen_object::draw() {
	if (!should_draw()) {
		return;
	}
	ne::transform3f draw_transform = transform;
	draw_transform.position.y -= bounce;
	draw_transform.scale.x += bounce / 8.0f;
	draw_transform.scale.y += bounce / 8.0f;
	draw_transform.position.x -= bounce / 8.0f;
	draw_transform.position.y -= bounce / 8.0f;
	ne::shader::set_transform(&draw_transform);
	still_quad().draw();
}

void enemy_slime_queen_object::explode(game_world* world) {
	enemy_chaser_object slime;
	slime.max_speed_normal = 1.0f;
	slime.max_speed_fast = 8.0f;
	slime.speed = 8.0f;
	slime.transform.position.xy = transform.position.xy + transform.scale.xy / 2.0f - 4.0f;
	int ticks = 1000;
	// Up
	slime.hold = { ticks, 0, 0, 0 };
	world->slime_enemies.push_back(slime);
	// Left
	slime.hold = { 0, ticks, 0, 0 };
	world->slime_enemies.push_back(slime);
	// Up left
	slime.hold = { ticks, ticks, 0, 0 };
	world->slime_enemies.push_back(slime);
	// Down
	slime.hold = { 0, 0, ticks, 0 };
	world->slime_enemies.push_back(slime);
	// Down left
	slime.hold = { 0, ticks, ticks, 0 };
	world->slime_enemies.push_back(slime);
	// Right
	slime.hold = { 0, 0, 0, ticks };
	world->slime_enemies.push_back(slime);
	// Up right
	slime.hold = { ticks, 0, 0, ticks };
	world->slime_enemies.push_back(slime);
	// Down right
	slime.hold = { 0, ticks, 0, ticks };
	world->slime_enemies.push_back(slime);
	audio.slime.play(50);
}

item_object::item_object() {
	random_bounce = ne::random_float(0.0f, 10000.0f);
}

void item_object::update(game_world* world) {
	bounce = std::sin((float)ne::ticks() / 200000.0f + random_bounce) * 2.0f;
}

void item_object::draw() {
	transform.scale.xy = ne::texture::bound()->size.to<float>();
	ne::transform3f draw_transform = transform;
	draw_transform.position.y -= bounce;
	draw_transform.scale.x += bounce / 8.0f;
	draw_transform.scale.y += bounce / 8.0f;
	draw_transform.position.x -= bounce / 8.0f;
	draw_transform.position.y -= bounce / 8.0f;
	ne::shader::set_transform(&draw_transform);
	still_quad().draw();
}

spike_object::spike_object() {
	transform.scale.xy = textures.spike.frame_size().to<float>();
	animation.fps = 5.0f;
	hearts = 15;
}

void spike_object::update(game_world* world) {
	
}

void spike_object::draw() {
	if (!should_draw()) {
		return;
	}
	ne::shader::set_transform(&transform);
	animation.draw();
}

artery_object::artery_object() {
	animation.fps = 0.0f;
	transform.scale.xy = textures.artery.frame_size().to<float>();
	is_flipped = ne::random_chance(0.45f);
	hearts = 5;
}

void artery_object::update(game_world* world) {
	
}

void artery_object::draw() {
	if (!should_draw()) {
		return;
	}
	ne::transform3f draw_transform = transform;
	if (is_flipped) {
		draw_transform.position.x += transform.scale.width;
		draw_transform.scale.width = -transform.scale.width;
	}
	ne::shader::set_transform(&draw_transform);
	animation.frame = type;
	animation.sub_frame = (float)type;
	animation.draw(false);
}

zindo_blood_object::zindo_blood_object() {
	hearts = 10;
	animation.fps = 5.0f + ne::random_float(5.0f);
	transform.scale.xy = textures.artery.frame_size().to<float>();
	last_shot.start();
}

void zindo_blood_object::update(game_world* world) {
	if (animation.frame > 3 && last_shot.milliseconds() > 1000) {
		world->bullets.push_back({ transform, ne::deg_to_rad(90.0f), false, BULLET_BLOOD });
		last_shot.start();
	}
}

void zindo_blood_object::draw() {
	if (!should_draw()) {
		return;
	}
	ne::shader::set_transform(&transform);
	animation.draw();
}

virus_object::virus_object() {
	animation.fps = 5.0f;
	transform.scale.xy = textures.virus.frame_size().to<float>();
	hearts = 20;
	waiter.start();
	made_sound.start();
}

void virus_object::update(game_world* world) {
	if (waiter.milliseconds() < 3000) {
		return;
	}
	angle += 0.25f;
	if (angle >= 360.0f) {
		angle = 0.0f;
	}
	if (made_sound.milliseconds() > 3000 + ne::random_int(3000)) {
		audio.beam.play(10);
		made_sound.start();
	}
	ne::transform3f origin = transform;
	origin.position.y -= 16.0f;
	world->bullets.push_back({ origin, ne::deg_to_rad(angle), false, BULLET_LASER });
	world->bullets.back().max_speed = 16.0f;
	world->bullets.back().type = BULLET_LASER;
}

void virus_object::draw() {
	if (!should_draw()) {
		return;
	}
	textures.virus.bind();
	ne::shader::set_transform(&transform);
	still_quad().bind();
	still_quad().draw();

	textures.flame_boost.bind();
	ne::transform3f flame_transform = transform;
	flame_transform.scale.xy = textures.flame_boost.frame_size().to<float>();
	
	flame_transform.position.x += transform.scale.width / 2.0f - flame_transform.scale.width / 2.0f;
	flame_transform.position.y += transform.scale.height - flame_transform.scale.height + 4.0f;

	ne::shader::set_transform(&flame_transform);
	animated_quad().bind();
	animation.draw();
}

neuron_object::neuron_object() {
	transform.scale.xy = textures.neuron.frame_size().to<float>();
	hearts = 10;
}

void neuron_object::update(game_world* world) {
	
}

void neuron_object::draw() {
	if (!should_draw()) {
		return;
	}
	ne::shader::set_transform(&transform);
	still_quad().draw();
}

eye_boss_object::eye_boss_object() {
	transform.scale.xy = textures.eye_boss.frame_size().to<float>();
	hearts = 120;
}

void eye_boss_object::update(game_world* world) {
	mace_angle += 0.1f;
}

void eye_boss_object::draw() {
	if (should_draw()) {
		textures.eye_boss.bind();
		ne::shader::set_transform(&transform);
		still_quad().draw();
	}
	ne::transform3f mace = transform;
	mace.scale.xy = textures.mace.size.to<float>();
	mace.position.x += textures.eye_boss.size.to<float>().width - 4.0f;
	mace.position.y += 16.0f;
	mace.rotation.z = ne::deg_to_rad(mace_angle);
	textures.mace.bind();
	ne::shader::set_transform(&mace);
	still_quad().draw();
}
