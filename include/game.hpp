#pragma once

#include "world.hpp"

#include <engine.hpp>
#include <camera.hpp>
#include <ui.hpp>

class game_state : public ne::program_state {
public:

	bool game_over = false;

	ne::font_text score_label;
	ne::font_text game_over_label;

	ne::ortho_camera camera;
	ne::ortho_camera ui_camera;
	game_world world;

	game_state();
	~game_state() override;

	void update() override;
	void draw() override;

private:

	ne::debug_info debug;

};

ne::drawing_shape& still_quad();
ne::drawing_shape& animated_quad();
