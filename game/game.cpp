#include "game.h"

#include "animation/animation_manager.h"
#include "audio/adaptive_music_manager.h"
#include "audio/audio_manager.h"
#include "cvars/cvars.h"
#include "display/display_manager.h"
#include "fmod_studio.hpp"
#include "gameplay/gameplay_manager.h"
#include "input/input_manager.h"
#include "render/transparent_elements/ui_manager.h"

#include "physics/physics_manager.h"

AutoCVarInt cvar_controlling_agent("game.controlling_agent", "Controlling agent", 1, CVarFlags::EditCheckbox);

void gui_setup() {
	SPDLOG_INFO("Initializing GUI scenes");

	auto &rm = ResourceManager::get();
	auto button_lit = rm.load_texture("button_lit_1.ktx2");
	auto button_unlit = rm.load_texture("button_unlit_1.ktx2");
	auto button_lit_sq = rm.load_texture("button_lit_square.ktx2");
	auto button_unlit_sq = rm.load_texture("button_unlit_square.ktx2");
	auto anchor = rm.load_texture("anchor_debug.ktx2");

	auto &ui_manager = UIManager::get();

	ui_manager.create_ui_scene("gui_test");
	ui_manager.activate_ui_scene("gui_test");

	auto &billboard_test = ui_manager.add_ui_image("gui_test", "billboard_test");
	billboard_test.texture = button_unlit_sq;
	billboard_test.is_screen_space = false;
	billboard_test.is_billboard = true;
	billboard_test.position = glm::vec3(0.0f);
	billboard_test.size = glm::vec2(2.0f, 2.0f);

	ui_manager.add_as_root("gui_test", "billboard_test");

	auto &button_anchor = ui_manager.add_ui_anchor("gui_test", "buttons_anchor");
	button_anchor.x = 0.25f;
	button_anchor.y = 0.5f;
	ui_manager.add_as_root("gui_test", "buttons_anchor");

	auto &play_button = ui_manager.add_ui_button("gui_test", "play_button", "button_hover", "button");
	play_button.texture = button_unlit;
	play_button.hover_texture = button_lit;
	play_button.position = glm::vec3(0.0f, 200.0f, 0.0f);
	play_button.size = glm::vec2(300.0f, 300.0f);
	play_button.text = "Play";
	button_anchor.add_child(play_button);

	auto &options_button = ui_manager.add_ui_button("gui_test", "options_button", "button_hover", "button");
	options_button.texture = button_unlit;
	options_button.hover_texture = button_lit;
	options_button.position = glm::vec3(0.0f, 0.0f, 0.0f);
	options_button.size = glm::vec2(300.0f, 300.0f);
	options_button.text = "Options";
	button_anchor.add_child(options_button);

	auto &credits_button = ui_manager.add_ui_button("gui_test", "credits_button", "button_hover", "button");
	credits_button.texture = button_unlit;
	credits_button.hover_texture = button_lit;
	credits_button.position = glm::vec3(0.0f, -200.0f, 0.0f);
	credits_button.size = glm::vec2(300.0f, 300.0f);
	credits_button.text = "Credits";
	button_anchor.add_child(credits_button);

	auto &title_root = ui_manager.add_ui_anchor("gui_test", "title_root");
	title_root.x = 0.75f;
	title_root.y = 0.5f;
	ui_manager.add_as_root("gui_test", "title_root");

	auto &title = ui_manager.add_ui_text("gui_test", "title");
	title.position = glm::vec3(0.0f, 0.0f, 0.0f);
	title.text = "Silence";
	title_root.add_child(title);

	auto &credits_root = ui_manager.add_ui_anchor("gui_test", "credits_root");
	credits_root.x = 0.5f;
	credits_root.y = 0.5f;
	credits_root.display = false;
	ui_manager.add_as_root("gui_test", "credits_root");

	auto &team_credits = ui_manager.add_ui_text("gui_test", "team_credits");
	team_credits.position = glm::vec3(0.0f, 100.0f, 0.0f);
	team_credits.text = "Made by: Silence Team";
	team_credits.size.x = 1.0f;
	credits_root.add_child(team_credits);

	auto &fmod_credits = ui_manager.add_ui_text("gui_test", "fmod_credits");
	fmod_credits.position = glm::vec3(0.0f, -100.0f, 0.0f);
	fmod_credits.text = "Sound achieved with FMOD Studio API";
	fmod_credits.size.x = 0.5f;
	credits_root.add_child(fmod_credits);

	auto &back_button_root = ui_manager.add_ui_anchor("gui_test", "back_button_root");
	back_button_root.x = 0.5f;
	back_button_root.y = 0.5f;
	back_button_root.display = false;
	ui_manager.add_as_root("gui_test", "back_button_root");

	auto &back_button = ui_manager.add_ui_button("gui_test", "back_button", "button_hover", "button");
	back_button.texture = button_unlit;
	back_button.hover_texture = button_lit;
	back_button.position = glm::vec3(0.0f, -200.0f, 0.0f);
	back_button.size = glm::vec2(300.0f, 300.0f);
	back_button.text = "Back";
	back_button_root.add_child(back_button);

	auto &options_root = ui_manager.add_ui_anchor("gui_test", "options_root");
	options_root.x = 0.5f;
	options_root.y = 0.5f;
	options_root.display = false;
	ui_manager.add_as_root("gui_test", "options_root");

	auto &master_volume = ui_manager.add_ui_text("gui_test", "master_volume");
	master_volume.position = glm::vec3(0.0f, 150.0f, 0.0f);
	master_volume.size.x = 0.66f;
	master_volume.text = "Master Volume";
	options_root.add_child(master_volume);

	for (int i = 0; i < 10; i++) {
		auto &square = ui_manager.add_ui_image("gui_test", "volume_meter_" + std::to_string(i));
		square.size = glm::vec2(25.0f, 25.0f);
		square.color = glm::vec4(0.0f);
		square.position = glm::vec3(-135.0f + (float)i * 30.0f, 0.0f, 0.0f);
		options_root.add_child(square);
	}

	auto &plus_button = ui_manager.add_ui_button("gui_test", "plus_button", "button_hover_square", "button_square");
	plus_button.texture = button_unlit_sq;
	plus_button.hover_texture = button_lit_sq;
	plus_button.position = glm::vec3(200.0f, 0.0f, 0.0f);
	plus_button.size = glm::vec2(75.0f, 75.0f);
	plus_button.text = "high";
	plus_button.text_scale = 0.66f;
	options_root.add_child(plus_button);

	auto &minus_button = ui_manager.add_ui_button("gui_test", "minus_button", "button_hover_square", "button_square");
	minus_button.texture = button_unlit_sq;
	minus_button.hover_texture = button_lit_sq;
	minus_button.position = glm::vec3(-200.0f, 0.0f, 0.0f);
	minus_button.size = glm::vec2(75.0f, 75.0f);
	minus_button.text = "low";
	minus_button.text_scale = 0.66f;
	options_root.add_child(minus_button);

	SPDLOG_INFO("GUI setup complete");
}

// didn't know where to put this so i guess here is fine for now
void ui_update() {
	auto &ui_manager = UIManager::get();
	auto &plus_button = ui_manager.get_ui_button("gui_test", "plus_button");
	auto &minus_button = ui_manager.get_ui_button("gui_test", "minus_button");
	auto &credits_button = ui_manager.get_ui_button("gui_test", "credits_button");
	auto &options_button = ui_manager.get_ui_button("gui_test", "options_button");
	auto &back_button = ui_manager.get_ui_button("gui_test", "back_button");

	auto &credits_root = ui_manager.get_ui_anchor("gui_test", "credits_root");
	auto &options_root = ui_manager.get_ui_anchor("gui_test", "options_root");
	auto &back_button_root = ui_manager.get_ui_anchor("gui_test", "back_button_root");
	auto &root = ui_manager.get_ui_anchor("gui_test", "buttons_anchor");
	auto &title_root = ui_manager.get_ui_anchor("gui_test", "title_root");

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

	FMOD::Studio::Bus *master_bus;
	AudioManager::get().get_system()->getBus("bus:/", &master_bus);

	float volume;
	master_bus->getVolume(&volume);
	for (int i = 0; i < 10; i++) {
		auto &volume_meter = ui_manager.get_ui_image("gui_test", "volume_meter_" + std::to_string(i));
		if ((i / 10.0f) < volume) {
			volume_meter.color = glm::vec4(1.0f);
		} else {
			volume_meter.color = glm::vec4(0.0f);
		}
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

void input_setup() {
	InputManager &input_manager = InputManager::get();
	input_manager.add_action("move_forward");
	input_manager.add_key_to_action("move_forward", InputKey::W);
	input_manager.add_action("move_backward");
	input_manager.add_key_to_action("move_backward", InputKey::S);
	input_manager.add_action("move_left");
	input_manager.add_key_to_action("move_left", InputKey::A);
	input_manager.add_action("move_right");
	input_manager.add_key_to_action("move_right", InputKey::D);
	input_manager.add_action("move_up");
	input_manager.add_key_to_action("move_up", InputKey::E);
	input_manager.add_action("move_down");
	input_manager.add_key_to_action("move_down", InputKey::Q);
	input_manager.add_action("move_faster");
	input_manager.add_key_to_action("move_faster", InputKey::LEFT_SHIFT);
	input_manager.add_action("agent_interact");
	input_manager.add_key_to_action("agent_interact", InputKey::E);
	input_manager.add_key_to_action("agent_interact", InputKey::GAMEPAD_BUTTON_A);

	input_manager.add_action("control_camera");
	input_manager.add_key_to_action("control_camera", InputKey::MOUSE_RIGHT);

	input_manager.add_action("agent_move_forward");
	input_manager.add_key_to_action("agent_move_forward", InputKey::W);

	input_manager.add_action("agent_move_backward");
	input_manager.add_key_to_action("agent_move_backward", InputKey::S);

	input_manager.add_action("agent_move_left");
	input_manager.add_key_to_action("agent_move_left", InputKey::A);

	input_manager.add_action("agent_move_right");
	input_manager.add_key_to_action("agent_move_right", InputKey::D);

	input_manager.add_action("agent_crouch");
	input_manager.add_key_to_action("agent_crouch", InputKey::C);
	input_manager.add_key_to_action("agent_crouch", InputKey::LEFT_CONTROL);

	//add actions to arrows
	input_manager.add_action("hacker_move_forward");
	input_manager.add_key_to_action("hacker_move_forward", InputKey::UP);
	input_manager.add_key_to_action("hacker_move_forward", InputKey::GAMEPAD_LEFT_STICK_Y_POSITIVE);

	input_manager.add_action("hacker_move_backward");
	input_manager.add_key_to_action("hacker_move_backward", InputKey::DOWN);
	input_manager.add_key_to_action("hacker_move_backward", InputKey::GAMEPAD_LEFT_STICK_Y_NEGATIVE);

	input_manager.add_action("hacker_move_left");
	input_manager.add_key_to_action("hacker_move_left", InputKey::LEFT);
	input_manager.add_key_to_action("hacker_move_left", InputKey::GAMEPAD_LEFT_STICK_X_NEGATIVE);

	input_manager.add_action("hacker_move_right");
	input_manager.add_key_to_action("hacker_move_right", InputKey::RIGHT);
	input_manager.add_key_to_action("hacker_move_right", InputKey::GAMEPAD_LEFT_STICK_X_POSITIVE);

	input_manager.add_action("hacker_look_up");
	input_manager.add_key_to_action("hacker_look_up", InputKey::GAMEPAD_RIGHT_STICK_Y_POSITIVE);

	input_manager.add_action("hacker_look_down");
	input_manager.add_key_to_action("hacker_look_down", InputKey::GAMEPAD_RIGHT_STICK_Y_NEGATIVE);

	input_manager.add_action("hacker_look_left");
	input_manager.add_key_to_action("hacker_look_left", InputKey::GAMEPAD_RIGHT_STICK_X_NEGATIVE);

	input_manager.add_action("hacker_look_right");
	input_manager.add_key_to_action("hacker_look_right", InputKey::GAMEPAD_RIGHT_STICK_X_POSITIVE);

	input_manager.add_action("toggle_debug_mode");
	input_manager.add_key_to_action("toggle_debug_mode", InputKey::ESCAPE);

	input_manager.add_action("mouse_left");
	input_manager.add_key_to_action("mouse_left", InputKey::MOUSE_LEFT);

	input_manager.add_action("mouse_right");
	input_manager.add_key_to_action("mouse_right", InputKey::MOUSE_RIGHT);

	input_manager.add_action("control_agent");
	input_manager.add_key_to_action("control_agent", InputKey::F1);

	input_manager.add_action("control_hacker");
	input_manager.add_key_to_action("control_hacker", InputKey::F2);

	input_manager.add_action("toggle_splitscreen");
	input_manager.add_key_to_action("toggle_splitscreen", InputKey::F3);

	input_manager.add_action("reload_scene");
	input_manager.add_key_to_action("reload_scene", InputKey::F4);
}

void handle_camera(DebugCamera &cam, float dt) {
	InputManager &input_manager = InputManager::get();
	float forward = input_manager.get_axis("move_backward", "move_forward");
	float right = input_manager.get_axis("move_left", "move_right");
	float up = input_manager.get_axis("move_down", "move_up");

	if (input_manager.is_action_pressed("move_faster")) {
		dt *= 3.0f;
	}

	cam.move_forward(forward * dt);
	cam.move_right(right * dt);
	cam.move_up(up * dt);

	glm::vec2 mouse_delta = input_manager.get_mouse_delta();
	cam.rotate(mouse_delta.x * dt, mouse_delta.y * dt);
}

void Game::startup() {
	Engine::startup();

	AdaptiveMusicManager::get().play();
	GameplayManager::get().enable();

	input_setup();
	// gui_setup();

	// Disable debug stuff on startup
	CVarSystem::get()->set_int_cvar("debug_draw.frustum.draw", 0);
	CVarSystem::get()->set_int_cvar("debug_draw.collision.draw", 0);
	CVarSystem::get()->set_int_cvar("debug_draw.lights.draw", 0);
	CVarSystem::get()->set_int_cvar("debug_camera.use", 0);
	CVarSystem::get()->set_int_cvar("render.splitscreen", 1);
}

void Game::shutdown() {
	Engine::shutdown();
}

void traverse_bsp_tree(BSPNode *node, std::vector<BSPNode> &nodes) {
	if (node == nullptr) {
		return;
	}

	nodes.push_back(*node);
	traverse_bsp_tree(node->front.get(), nodes);
	traverse_bsp_tree(node->back.get(), nodes);
}

void Game::custom_update(float dt) {
	ZoneScopedN("Custom Update");
	DisplayManager &display_manager = DisplayManager::get();
	InputManager &input_manager = InputManager::get();

	// Reforward input events if requested
	if (input_manager.is_action_just_pressed("control_agent")) {
		input_manager.remove_key_from_action("hacker_move_forward", InputKey::W);
		input_manager.remove_key_from_action("hacker_move_backward", InputKey::S);
		input_manager.remove_key_from_action("hacker_move_left", InputKey::A);
		input_manager.remove_key_from_action("hacker_move_right", InputKey::D);

		input_manager.add_key_to_action("agent_move_forward", InputKey::W);
		input_manager.add_key_to_action("agent_move_backward", InputKey::S);
		input_manager.add_key_to_action("agent_move_left", InputKey::A);
		input_manager.add_key_to_action("agent_move_right", InputKey::D);

		cvar_controlling_agent.set(1);
	}
	if (input_manager.is_action_just_pressed("control_hacker")) {
		input_manager.remove_key_from_action("agent_move_forward", InputKey::W);
		input_manager.remove_key_from_action("agent_move_backward", InputKey::S);
		input_manager.remove_key_from_action("agent_move_left", InputKey::A);
		input_manager.remove_key_from_action("agent_move_right", InputKey::D);

		input_manager.add_key_to_action("hacker_move_forward", InputKey::W);
		input_manager.add_key_to_action("hacker_move_backward", InputKey::S);
		input_manager.add_key_to_action("hacker_move_left", InputKey::A);
		input_manager.add_key_to_action("hacker_move_right", InputKey::D);

		cvar_controlling_agent.set(0);
	}
	if (input_manager.is_action_just_pressed("toggle_splitscreen")) {
		CVarSystem::get()->set_int_cvar("render.splitscreen", !*CVarSystem::get()->get_int_cvar("render.splitscreen"));
	}

	// Resize scene's framebuffers if necessary
	static glm::vec2 last_framebuffer_size = glm::vec2(100, 100);
	glm::vec2 framebuffer_size = display_manager.get_framebuffer_size();
	if (framebuffer_size != last_framebuffer_size) {
		last_framebuffer_size = framebuffer_size;
		for (auto &scene : scenes) {
			scene->get_render_scene().resize_framebuffer(framebuffer_size.x, framebuffer_size.y);
		}
	}

	// Handle camera
	if (input_manager.is_action_just_pressed("toggle_debug_mode")) {
		in_debug_mode = !in_debug_mode;
		CVarSystem::get()->set_int_cvar("debug_draw.frustum.draw", in_debug_mode);
		CVarSystem::get()->set_int_cvar("debug_camera.use", in_debug_mode);
		show_cvar_editor = in_debug_mode;

		if (in_debug_mode) {
			DebugCamera &cam = get_active_scene().get_render_scene().debug_camera;
			cam.set_transform(get_active_scene().get_render_scene().left_camera_transform);
		}
	}

	//ui_update();

	// get imgui io
	ImGuiIO &io = ImGui::GetIO();
	if (in_debug_mode) {
		if (input_manager.is_action_pressed("control_camera") && io.WantCaptureMouse == false) {
			display_manager.capture_mouse(true);
			DebugCamera &cam = get_active_scene().get_render_scene().debug_camera;
			handle_camera(cam, dt);
		} else {
			display_manager.capture_mouse(false);
		}
	} else {
		display_manager.capture_mouse(true);
	}

	// ImGui
	if (in_debug_mode) {
		ImGui::Begin("Debug Menu");
		ImGui::Text("Press ESC to get back to the game");
		ImGui::Text("%f FPS (%.2f ms)", 1.0f / dt, 1000.0f * dt);
		ImGui::End();
	}

	if (input_manager.is_action_just_pressed("reload_scene")) {
		get_active_scene().load_from_file("resources/scenes/level_1.scn");
		AnimationManager::get().animation_map.clear();
	}

	// BSP Vizualization, for now it's left in here, but could be moved to system in the future
	//
	// bsp_tree = CollisionSystem::build_tree(world, entities, 10);

	BSPNode *node = get_active_scene().bsp_tree.get();
	std::vector<BSPNode> nodes;
	traverse_bsp_tree(node, nodes);

	ImGui::Begin("BSP Tree");

	for (auto &node : nodes) {
		// calculate rotation from normal
		glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 axis = glm::cross(up, node.plane.normal);
		float angle = glm::acos(glm::dot(up, node.plane.normal));
		glm::quat rotation = glm::angleAxis(angle, axis);

		std::string info = "Node: " + std::to_string(node.entities.size());
		ImGui::Text("%s", info.c_str());

		if (node.front || node.back) {
			SPDLOG_INFO("normal {}", glm::to_string(node.plane.normal));
			SPDLOG_INFO("point {}", glm::to_string(node.plane.point));
			SPDLOG_INFO("entities {}", node.entities.size());
			get_active_scene().get_render_scene().debug_draw.draw_box(
					node.plane.point, rotation, glm::vec3(20.0f, 20.0f, 0.1f), glm::vec3(1.0f, 0.0f, 0.0f));
			// draw arrow
			get_active_scene().get_render_scene().debug_draw.draw_arrow(
					node.plane.point, node.plane.point + node.plane.normal, glm::vec3(1.0f, 0.0f, 0.0f));
		}
	}

	ImGui::End();
}
