#include <GLEW/glew.h>
#include <SDL/SDL.h>

#include "game.hpp"
#include "assets.hpp"
#include "menu.hpp"

#include <engine.hpp>
#include <window.hpp>
#include <graphics.hpp>

void start() {
	// Black background color.
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

	// Start the game in maximised window mode.
	ne::maximise_window();

	// Load all the assets. (blocks)
	load_assets();

	// Create shapes.
	still_quad().create();
	still_quad().make_quad();
	animated_quad().create();
	animated_quad().make_quad();

	// Turn off VSync.
	// TODO: Ensure this is set to 'sync' on releases.
	ne::set_swap_interval(ne::swap_interval::immediate);
	//ne::set_swap_interval(ne::swap_interval::sync);

	// Start the game.
	ne::swap_state<menu_state>();
}

void stop() {
	destroy_assets();
}

int main(int argc, char** argv) {
	ne::start_engine("Ludum Dare 41", 800, 600);
	return ne::enter_loop(start, stop);
}
