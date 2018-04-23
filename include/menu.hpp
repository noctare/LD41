#pragma once

#include <engine.hpp>
#include <camera.hpp>
#include <ui.hpp>

class menu_state : public ne::program_state {
public:

	ne::ortho_camera camera;

	menu_state();
	~menu_state() override;

	void update() override;
	void draw() override;
	
private:

	ne::ui_button play1;
	ne::ui_button play2;
	ne::font_text select;

};

