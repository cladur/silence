#include "menu_test.h"
#include "audio/audio_manager.h"
#include "render/transparent_elements/ui/sprite_manager.h"
#include <render/render_scene.h>

void MenuTest::startup(RenderScene &scene) {
	auto res = AudioManager::get().get_system()->getBus("bus:/", &master_bus);
	SPDLOG_INFO("Got bus: {}", res);

	SpriteManager::get()->load_sprite_texture("button_hover", "button_lit_1.ktx2");
	SpriteManager::get()->load_sprite_texture("button", "button_unlit_1.ktx2");
	SpriteManager::get()->load_sprite_texture("button_hover_square", "button_lit_square.ktx2");
	SpriteManager::get()->load_sprite_texture("button_square", "button_unlit_square.ktx2");
	SpriteManager::get()->load_sprite_texture("anchor_debug", "anchor_debug.ktx2");

	billboard_test = UIImage(
			glm::vec3(0.0f, 0.0f, 0.0f),
			glm::vec2(2.0f, 2.0f),
			"button_hover_square");
	billboard_test.is_screen_space = false;
	billboard_test.is_billboard = true;

	root = UIAnchor(0.25f, 0.5f);

	play_button = UIButton(glm::vec3(0.0f, 200.0f, 0.0f), glm::vec2(300.0f, 300.0f), "Play", "PoltawskiNowy", "button");
	play_button.hover_texture_name = "button_hover";
	root.add_child(play_button);

	options_button = UIButton(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(300.0f, 300.0f), "Options", "PoltawskiNowy", "button");
	options_button.hover_texture_name = "button_hover";

	root.add_child(options_button);

	credits_button = UIButton(glm::vec3(0.0f, -200.0f, 0.0f), glm::vec2(300.0f, 300.0f), "Credits", "PoltawskiNowy", "button");
	credits_button.hover_texture_name = "button_hover";
	root.add_child(credits_button);

	title_root = UIAnchor(0.75f, 0.5f);
	title_root.draw_anchor = true;

	title_text = UIText(glm::vec3(0.0f, 0.0f, 0.0f), 1.4f, "Silence", "PoltawskiNowy");
	title_text.centered_x = true;
	title_text.centered_y = true;
	title_root.add_child(title_text);

	// Credits
	credits_root = UIAnchor(0.5f, 0.5f);
	credits_text = UIText(glm::vec3(0.0f, 0.0f, 0.0f), 1.2f, "Silence Team", "PoltawskiNowy");
	credits_text.centered_x = true;
	credits_text.centered_y = true;
	credits_root.display = false;
	credits_root.add_child(credits_text);

	// Back button
	back_button_root = UIAnchor(0.5f, 0.5f);
	back_button = UIButton(glm::vec3(0.0f, -200.0f, 0.0f), glm::vec2(300.0f, 300.0f), "Back", "PoltawskiNowy", "button");
	back_button.hover_texture_name = "button_hover";
	back_button_root.display = false;
	back_button_root.add_child(back_button);

	options_root = UIAnchor(0.5f, 0.5f);
	options_root.display = false;
	master_volume = UIText(glm::vec3(0.0f, 150.0f, 0.0f), 0.66f, "Master Volume", "PoltawskiNowy");
	master_volume.centered_x = true;
	master_volume.centered_y = true;
	options_root.add_child(master_volume);

	for (int i = 0; i < 10; i++) {
		UIImage square = UIImage(glm::vec3(-135.0f + i * 30.0f, 0.0f, 0.0f), glm::vec2(25.0f, 25.0f), "");
		square.color = glm::vec3(0.0f);
		volume_meter[i] = square;
		options_root.add_child(volume_meter[i]);
	}

	plus_button = UIButton(glm::vec3(200.0f, 0.0f, 0.0f), glm::vec2(75.0f, 75.0f), "high", "PoltawskiNowy", "button_square");
	plus_button.text_scale = 0.5f;

	plus_button.hover_texture_name = "button_hover_square";
	options_root.add_child(plus_button);

	minus_button = UIButton(glm::vec3(-200.0f, 0.0f, 0.0f), glm::vec2(75.0f, 75.0f), "low", "PoltawskiNowy", "button_square");

	minus_button.text_scale = 0.5f;
	minus_button.hover_texture_name = "button_hover_square";
	options_root.add_child(minus_button);

	// THE NECESSARY NEW PART (FOR NOW MAYBE)
	scene.ui_draw.ui_objects.push_back(&billboard_test);
	scene.ui_draw.ui_objects.push_back(&root);
	scene.ui_draw.ui_objects.push_back(&title_root);
	scene.ui_draw.ui_objects.push_back(&credits_root);
	scene.ui_draw.ui_objects.push_back(&back_button_root);
	scene.ui_draw.ui_objects.push_back(&options_root);
}

void MenuTest::update() {
	for (int i = 0; i < 10; i++) {
		float volume;
		master_bus->getVolume(&volume);
		if ((i / 10.0f) < volume) {
			volume_meter[i].color = glm::vec3(1.0f);
		} else {
			volume_meter[i].color = glm::vec3(0.0f);
		}
	}

	if (credits_button.clicked()) {
		credits_root.display = true;
		root.display = false;
		title_root.display = false;
		options_root.display = false;

		back_button_root.display = true;
	}

	if (options_button.clicked()) {
		options_root.display = true;

		root.display = false;
		title_root.display = false;
		credits_root.display = false;

		back_button_root.display = true;
	}

	if (back_button.clicked()) {
		root.display = true;
		title_root.display = true;

		credits_root.display = false;
		options_root.display = false;

		back_button_root.display = false;
	}

	if (plus_button.clicked()) {
		float volume;

		master_bus->getVolume(&volume);
		master_bus->setVolume(std::round(std::clamp(volume + 0.1f, 0.0f, 1.0f) * 10.0f) / 10.0f);
	}

	if (minus_button.clicked()) {
		float volume;
		master_bus->getVolume(&volume);
		master_bus->setVolume(std::round(std::clamp(volume - 0.1f, 0.0f, 1.0f) * 10.0f) / 10.0f);
	}
}

