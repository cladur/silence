#include "game.h"
#include "audio/audio_manager.h"
#include "cvars/cvars.h"
#include "display/display_manager.h"
#include "input/input_manager.h"
#include <imgui.h>

void input_setup() {
	InputManager &input_manager = InputManager::get();
	input_manager.add_action("move_forward");
	input_manager.add_key_to_action("move_forward", InputKey::W);
	input_manager.add_key_to_action("move_forward", InputKey::GAMEPAD_LEFT_STICK_Y_POSITIVE);
	input_manager.add_action("move_backward");
	input_manager.add_key_to_action("move_backward", InputKey::S);
	input_manager.add_key_to_action("move_backward", InputKey::GAMEPAD_LEFT_STICK_Y_NEGATIVE);
	input_manager.add_action("move_left");
	input_manager.add_key_to_action("move_left", InputKey::A);
	input_manager.add_key_to_action("move_left", InputKey::GAMEPAD_LEFT_STICK_X_NEGATIVE);
	input_manager.add_action("move_right");
	input_manager.add_key_to_action("move_right", InputKey::D);
	input_manager.add_key_to_action("move_right", InputKey::GAMEPAD_LEFT_STICK_X_POSITIVE);
	input_manager.add_action("move_up");
	input_manager.add_key_to_action("move_up", InputKey::E);
	input_manager.add_key_to_action("move_up", InputKey::GAMEPAD_BUTTON_A);
	input_manager.add_action("move_down");
	input_manager.add_key_to_action("move_down", InputKey::Q);
	input_manager.add_key_to_action("move_down", InputKey::GAMEPAD_BUTTON_B);
	input_manager.add_action("move_faster");
	input_manager.add_key_to_action("move_faster", InputKey::LEFT_SHIFT);

	input_manager.add_action("control_camera");
	input_manager.add_key_to_action("control_camera", InputKey::MOUSE_RIGHT);

	//add actions to arrows
	input_manager.add_action("forward");
	input_manager.add_key_to_action("forward", InputKey::UP);

	input_manager.add_action("backward");
	input_manager.add_key_to_action("backward", InputKey::DOWN);

	input_manager.add_action("left");
	input_manager.add_key_to_action("left", InputKey::LEFT);

	input_manager.add_action("right");
	input_manager.add_key_to_action("right", InputKey::RIGHT);

	input_manager.add_action("up");
	input_manager.add_key_to_action("up", InputKey::O);

	input_manager.add_action("down");
	input_manager.add_key_to_action("down", InputKey::L);

	input_manager.add_action("toggle_debug_mode");
	input_manager.add_key_to_action("toggle_debug_mode", InputKey::ESCAPE);

	input_manager.add_action("mouse_left");
	input_manager.add_key_to_action("mouse_left", InputKey::MOUSE_LEFT);
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

	input_setup();

	// Disable debug stuff on startup
	CVarSystem::get()->set_int_cvar("debug_draw.frustum.draw", 0);
	CVarSystem::get()->set_int_cvar("debug_draw.collision.draw", 0);
	CVarSystem::get()->set_int_cvar("debug_camera.use", 0);
}

void Game::shutdown() {
	Engine::shutdown();
}

void Game::custom_update(float dt) {
	DisplayManager &display_manager = DisplayManager::get();
	InputManager &input_manager = InputManager::get();

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
			cam.set_transform(get_active_scene().get_render_scene().camera_transform);
		}
	}

	//menu_test.update();

	// get imgui io
	ImGuiIO &io = ImGui::GetIO();
	if (in_debug_mode && input_manager.is_action_pressed("control_camera") && io.WantCaptureMouse == false) {
		display_manager.capture_mouse(true);
		DebugCamera &cam = get_active_scene().get_render_scene().debug_camera;
		handle_camera(cam, dt);
	} else {
		display_manager.capture_mouse(false);
	}

	// ImGui
	if (in_debug_mode) {
		ImGui::Begin("Debug Menu");
		ImGui::Text("Press ESC to get back to the game");
		ImGui::End();
	}
}
