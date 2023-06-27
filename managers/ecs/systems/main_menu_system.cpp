#include "main_menu_system.h"
#include "ecs/world.h"
#include <audio/audio_manager.h>
#include <display/display_manager.h>
#include <gameplay/gameplay_manager.h>
#include <render/transparent_elements/ui_manager.h>
#include <engine/scene.h>

void MainMenuSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<MainMenu>());
	world.set_system_component_whitelist<MainMenuSystem>(whitelist);
}

void MainMenuSystem::update(World &world, float dt) {
	ZoneScopedN("MainMenuSystem::update");
	for (auto const &entity : entities) {
		auto &menu = world.get_component<MainMenu>(entity);
		auto &gp = GameplayManager::get();
		if (menu.first_frame) {
			init_ui(menu);
			menu.first_frame = false;
		}
		gp.game_state = GameState::MAIN_MENU;
		DisplayManager::get().capture_mouse(false); // todo: change this to once, when the debug mode is gone

		CVarSystem::get()->set_int_cvar("render.splitscreen", 0);

		for (int i = 0; i < 10; i++) {
			float volume;
			menu.master_bus->getVolume(&volume);
			if ((i / 10.0f) < volume) {
				menu.volume_meter[i]->color = glm::vec4(1.0f);
			} else {
				menu.volume_meter[i]->color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			}
		}

		if (menu.play_button->clicked()) {
			SPDLOG_INFO("Play button clicked");
			gp.change_scene("Level");
			UIManager::get().deactivate_ui_scene(ui_name);
			break;
		}

		if (menu.options_button->clicked()) {
			SPDLOG_INFO("Options button clicked");
			menu.options_root->display = true;

			menu.root->display = false;
			menu.title_root->display = false;

			menu.back_button_root->display = true;
		}

		if (menu.back_button->clicked()) {
			menu.root->display = true;
			menu.title_root->display = true;

			menu.options_root->display = false;

			menu.back_button_root->display = false;
		}

		if (menu.quit_button->clicked()) {
			glfwSetWindowShouldClose(DisplayManager::get().window, GLFW_TRUE);
		}

		if (menu.plus_button->clicked()) {
			float volume;

			menu.master_bus->getVolume(&volume);
			menu.master_bus->setVolume(std::round(std::clamp(volume + 0.1f, 0.0f, 1.0f) * 10.0f) / 10.0f);
		}

		if (menu.minus_button->clicked()) {
			float volume;
			menu.master_bus->getVolume(&volume);
			menu.master_bus->setVolume(std::round(std::clamp(volume - 0.1f, 0.0f, 1.0f) * 10.0f) / 10.0f);
		}
	}
}

void MainMenuSystem::init_ui(MainMenu &menu) {

	SPDLOG_INFO("Initializing main menu UI");
	AudioManager::get().get_system()->getBus("bus:/", &menu.master_bus);
	auto &rm = ResourceManager::get();
	auto &ui = UIManager::get();
	ui_name = "main_menu_scene";

	ui.create_ui_scene(ui_name);
	ui.activate_ui_scene(ui_name);

	menu.root = &ui.add_ui_anchor(ui_name, "main_menu_root");
	menu.root->x = 0.25f;
	menu.root->y = 0.5f;
	menu.root->display = true;

	ui.add_as_root(ui_name, "main_menu_root");

	menu.play_button = &ui.add_ui_button(ui_name, "play_button", "", "");
	menu.play_button->position = glm::vec3(0.0f, 200.0f, 0.0f);
	menu.play_button->size = glm::vec2(300.0f, 300.0f);
	menu.play_button->text = "Play";
	menu.play_button->texture = rm.load_texture(asset_path("button_unlit_1.ktx2").c_str());
	menu.play_button->hover_texture = rm.load_texture(asset_path("button_lit_1.ktx2").c_str());
	menu.play_button->display = true;

	ui.add_to_root(ui_name, "play_button", "main_menu_root");

	menu.options_button = &ui.add_ui_button(ui_name, "options_button", "", "");
	menu.options_button->position = glm::vec3(0.0f, 0.0f, 0.0f);
	menu.options_button->size = glm::vec2(300.0f, 300.0f);
	menu.options_button->text = "Options";
	menu.options_button->texture = rm.load_texture(asset_path("button_unlit_1.ktx2").c_str());
	menu.options_button->hover_texture = rm.load_texture(asset_path("button_lit_1.ktx2").c_str());

	ui.add_to_root(ui_name, "options_button", "main_menu_root");

	menu.quit_button = &ui.add_ui_button(ui_name, "quit_button", "", "");
	menu.quit_button->position = glm::vec3(0.0f, -200.0f, 0.0f);
	menu.quit_button->size = glm::vec2(300.0f, 300.0f);
	menu.quit_button->text = "Quit";
	menu.quit_button->texture = rm.load_texture(asset_path("button_unlit_1.ktx2").c_str());
	menu.quit_button->hover_texture = rm.load_texture(asset_path("button_lit_1.ktx2").c_str());

	ui.add_to_root(ui_name, "quit_button", "main_menu_root");

	menu.title_root = &ui.add_ui_anchor(ui_name, "title_root");
	menu.title_root->x = 0.75f;
	menu.title_root->y = 0.75f;

	ui.add_as_root(ui_name, "title_root");

	menu.title_text = &ui.add_ui_text(ui_name, "title_text");
	menu.title_text->position = glm::vec3(0.0f, 0.0f, 0.0f);
	menu.title_text->size = glm::vec2(1.2f);
	menu.title_text->centered_x = true;
	menu.title_text->centered_y = true;
	menu.title_text->text = "Silence";

	ui.add_to_root(ui_name, "title_text", "title_root");

	menu.back_button_root = &ui.add_ui_anchor(ui_name, "back_button_root");
	menu.back_button_root->x = 0.5f;
	menu.back_button_root->y = 0.5f;
	menu.back_button_root->display = false;

	ui.add_as_root(ui_name, "back_button_root");

	menu.back_button = &ui.add_ui_button(ui_name, "back_button", "", "");
	menu.back_button->position = glm::vec3(0.0f, -200.0f, 0.0f);
	menu.back_button->size = glm::vec2(300.0f, 300.0f);
	menu.back_button->text = "Back";
	menu.back_button->texture = rm.load_texture(asset_path("button_unlit_1.ktx2").c_str());
	menu.back_button->hover_texture = rm.load_texture(asset_path("button_lit_1.ktx2").c_str());

	ui.add_to_root(ui_name, "back_button", "back_button_root");

	menu.options_root = &ui.add_ui_anchor(ui_name, "options_root");
	menu.options_root->x = 0.5f;
	menu.options_root->y = 0.5f;
	menu.options_root->display = false;

	ui.add_as_root(ui_name, "options_root");

	menu.options_text = &ui.add_ui_text(ui_name, "options_text");
	menu.options_text->position = glm::vec3(0.0f, 200.0f, 0.0f);
	menu.options_text->size = glm::vec2(1.0f);
	menu.options_text->centered_x = true;
	menu.options_text->centered_y = true;
	menu.options_text->text = "Options";

	ui.add_to_root(ui_name, "options_text", "options_root");

	for (int i = 0; i < 10; i++) {
		menu.volume_meter[i] = &ui.add_ui_image(ui_name, "square_" + std::to_string(i));
		menu.volume_meter[i]->position = glm::vec3(-200.0f + 40.0f * i, 0.0f, 0.0f);
		menu.volume_meter[i]->size = glm::vec2(25.0f, 25.0f);
		menu.volume_meter[i]->texture = Handle<Texture>(-1);

		ui.add_to_root(ui_name, "square_" + std::to_string(i), "options_root");
	}

	menu.plus_button = &ui.add_ui_button(ui_name, "plus_button", "", "");
	menu.plus_button->position = glm::vec3(275.0f, 0.0f, 0.0f);
	menu.plus_button->size = glm::vec2(50.0f, 50.0f);
	menu.plus_button->text = "+";
	menu.plus_button->texture = rm.load_texture(asset_path("button_unlit_square.ktx2").c_str());
	menu.plus_button->centered_x = true;
	menu.plus_button->centered_y = true;
	menu.plus_button->hover_texture = rm.load_texture(asset_path("button_lit_square.ktx2").c_str());

	ui.add_to_root(ui_name, "plus_button", "options_root");

	menu.minus_button = &ui.add_ui_button(ui_name, "minus_button", "", "");
	menu.minus_button->position = glm::vec3(-275.0f, 0.0f, 0.0f);
	menu.minus_button->size = glm::vec2(50.0f, 50.0f);
	menu.minus_button->text = "-";
	menu.minus_button->centered_x = true;
	menu.minus_button->centered_y = true;
	menu.minus_button->texture = rm.load_texture(asset_path("button_unlit_square.ktx2").c_str());
	menu.minus_button->hover_texture = rm.load_texture(asset_path("button_lit_square.ktx2").c_str());

	ui.add_to_root(ui_name, "minus_button", "options_root");

	auto &fmod_credits_root = ui.add_ui_anchor(ui_name, "fmod_credits_root");
	fmod_credits_root.x = 0.75f;
	fmod_credits_root.y = 0.1f;
	fmod_credits_root.display = true;

	ui.add_as_root(ui_name, "fmod_credits_root");

	auto &fmod_credits_image = ui.add_ui_image(ui_name, "fmod_credits_image");
	fmod_credits_image.position = glm::vec3(0.0f, 0.0f, 0.0f);
	fmod_credits_image.size = glm::vec2(256.0f);
	fmod_credits_image.texture = rm.load_texture(asset_path("fmod_logo.ktx2").c_str());
	fmod_credits_image.color = glm::vec4(1.0f);

	ui.add_to_root(ui_name, "fmod_credits_image", "fmod_credits_root");

	auto &fmod_credits_text1 = ui.add_ui_text(ui_name, "fmod_credits_text1");
	fmod_credits_text1.position = glm::vec3(0.0f, 128.0f, 0.0f);
	fmod_credits_text1.size = glm::vec2(0.4f);
	fmod_credits_text1.centered_x = true;
	fmod_credits_text1.centered_y = true;
	fmod_credits_text1.text = "Sound achieved with:";

	ui.add_to_root(ui_name, "fmod_credits_text1", "fmod_credits_root");

	auto &fmod_credits_text2 = ui.add_ui_text(ui_name, "fmod_credits_text2");
	fmod_credits_text2.position = glm::vec3(0.0f, 100.0f, 0.0f);
	fmod_credits_text2.size = glm::vec2(0.4f);
	fmod_credits_text2.centered_x = true;
	fmod_credits_text2.centered_y = true;
	fmod_credits_text2.text = "FMOD Studio by Firelight Technologies Pty Ltd.";

	ui.add_to_root(ui_name, "fmod_credits_text2", "fmod_credits_root");
}

MainMenuSystem::~MainMenuSystem() {
	UIManager::get().delete_ui_scene(ui_name);
}