#include "display_manager.h"
#include "render_manager.h"

#include "../core/input/input_manager.h"
#include "../core/input/multiplatform_input.h"

#include "magic_enum.hpp"
#include "spdlog/spdlog.h"

RenderManager render_manager;
DisplayManager display_manager;
InputManager *input_manager;

void default_mappings(){
	input_manager->map_input_to_action(InputKey::W, InputAction{ .action_name = "Forward", .scale = 1.f });
	input_manager->map_input_to_action(InputKey::S, InputAction{ .action_name = "Backward", .scale = -1.f });
	input_manager->map_input_to_action(InputKey::A, InputAction{ .action_name = "Left", .scale = -1.f });
	input_manager->map_input_to_action(InputKey::D, InputAction{ .action_name = "Right", .scale = 1.f });
	input_manager->map_input_to_action(InputKey::MOUSE_LEFT, InputAction{ .action_name = "MouseClick", .scale = 1.f });
	input_manager->map_input_to_action(InputKey::L_STICK_X, InputAction{ .action_name = "LAxisX", .scale = 1.f, .deadzone = 0.1f });
	input_manager->map_input_to_action(InputKey::L_STICK_Y, InputAction{ .action_name = "LAxisY", .scale = 1.f, .deadzone = 0.1f });
}

int main() {
	SPDLOG_INFO("Starting up engine systems...");

	auto dm_ret = display_manager.startup();
	if (dm_ret == DisplayManager::Status::Ok) {
		SPDLOG_INFO("Initialized display manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the display manager. Status: ({}) {}", magic_enum::enum_integer(dm_ret),
				magic_enum::enum_name(dm_ret));
		return -1;
	}

	auto rm_ret = render_manager.startup(display_manager);
	if (rm_ret == RenderManager::Status::Ok) {
		SPDLOG_INFO("Initialized render manager");
	} else {
		SPDLOG_ERROR("Failed to initialize the render manager. Status: ({}) {}", magic_enum::enum_integer(rm_ret),
				magic_enum::enum_name(rm_ret));
		return -1;
	}

	//InputManager setup
	input_manager = new InputManager;
	display_manager.setup_input();

	//Map inputs
	default_mappings();

	// Run the game.
	bool should_run = true;
	while (should_run) {
		// GAME LOGIC
		display_manager.poll_events();
		input_manager->process_input();

		//SPDLOG_INFO(input_manager->get_axis("Left","Right"));

		if (display_manager.window_should_close()) {
			should_run = false;
		}
	}

	// Shut everything down, in reverse order.
	SPDLOG_INFO("Shutting down engine subsystems...");
	render_manager.shutdown();
	display_manager.shutdown();

	return 0;
}
