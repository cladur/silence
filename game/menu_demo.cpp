#include "menu_demo.h"
#include "opengl/transparent_elements/ui/sprite_manager.h"

MenuDemo::MenuDemo() {
	SpriteManager::get()->load_sprite_texture("button_hover", "button_lit_1.ktx2");
	SpriteManager::get()->load_sprite_texture("button", "button_unlit_1.ktx2");

	root = UIAnchor(0.25f, 0.5f);
	play_button = UIButton(
			glm::vec3(0.0f, 200.0f, 0.0f),
			glm::vec2(300.0f, 300.0f),
			"Play",
			"two",
			"button");
	play_button.hover_texture_name = "button_hover";
	root.add_child(play_button);
	options_button = UIButton(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec2(300.0f, 300.0f),
			"Options",
			"two",
			"button");
	options_button.hover_texture_name = "button_hover";
	root.add_child(options_button);
	credits_button = UIButton(
			glm::vec3(0.0f, -200.0f, 0.0f),
			glm::vec2(300.0f, 300.0f),
			"Credits",
			"two",
			"button");
	credits_button.hover_texture_name = "button_hover";
	root.add_child(credits_button);
}


void MenuDemo::draw() {
	root.draw();
}

