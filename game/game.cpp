#include "game.h"
#include "display/display_manager.h"
#include "input/input_manager.h"

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

	input_manager.add_action("toggle_camera_control");
	input_manager.add_key_to_action("toggle_camera_control", InputKey::ESCAPE);
}

void handle_camera(Camera &cam, float dt) {
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
	if (input_manager.is_action_just_pressed("toggle_camera_control")) {
		controlling_camera = !controlling_camera;
		display_manager.capture_mouse(controlling_camera);
	}

	if (controlling_camera) {
		Camera &cam = get_active_scene().camera;
		handle_camera(cam, dt);
	}

	// ImGui
	if (!controlling_camera) {
		ImGui::Begin("Game");
		ImGui::Text("Press ESC to control camera");
		ImGui::Checkbox("Show cvar editor", &show_cvar_editor);
		ImGui::End();
	}
}
