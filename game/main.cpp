#include "display_manager.h"
#include "render_manager.h"

#include "../core/input/input_manager.h"
#include "../core/input/multiplatform_input.h"

#include "magic_enum.hpp"
#include "spdlog/spdlog.h"

RenderManager render_manager;
DisplayManager display_manager;
InputManager *input_manager;

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
	MultiplatformInput input{};
	glfwSetWindowUserPointer(display_manager.window, &input);
	glfwSetKeyCallback(display_manager.window, [](GLFWwindow *window, int key, int scancode, int action, int mods) {
		auto *input = static_cast<MultiplatformInput *>(glfwGetWindowUserPointer(window));
		float value = 0.f;
		switch (action) {
			case GLFW_PRESS:
			case GLFW_REPEAT:
				value = 1.f;
				break;
			case GLFW_RELEASE:
				break;
			default:
				value = 0.f;
		}
		input->update_keyboard_state(key, value);
	});

	glfwSetMouseButtonCallback(display_manager.window, [](GLFWwindow *window, int button, int action, int mods) {
		auto *input = static_cast<MultiplatformInput *>(glfwGetWindowUserPointer(window));
		if (input) {
			input->update_mouse_state(button, action == GLFW_PRESS ? 1.f : 0.f);
		}
	});

	if (input_manager) {
		input_manager->register_device(InputDevice{

				.Type = InputDeviceType::MOUSE,
				.Index = 0,
				.StateFunc = std::bind(&MultiplatformInput::get_mouse_state, &input, std::placeholders::_1) });

		input_manager->register_device(InputDevice{

				.Type = InputDeviceType::KEYBOARD,
				.Index = 0,
				.StateFunc = std::bind(&MultiplatformInput::get_keyboard_state, &input, std::placeholders::_1) });

		//Map inputs
		input_manager->map_input_to_action(InputKey::A, InputAction{ .action_name = "Strife", .scale = -1.f });
		input_manager->map_input_to_action(InputKey::D, InputAction{ .action_name = "Strife", .scale = 1.f });
		input_manager->map_input_to_action(InputKey::MOUSE_LEFT, InputAction{ .action_name = "Click", .scale = 1.f });

		input_manager->register_action_callback("Strife",
				InputManager::ActionCallback{ .callback_reference = "Reference",
						.func = [](InputSource source, int source_index, float value) {
							printf("%f \n", value);
							return true;
						} });
		input_manager->register_action_callback("Click",
				InputManager::ActionCallback{
						.callback_reference = "Mouse", .func = [](InputSource source, int source_index, float value) {
							if (value == 1) {
								printf("Clicked\n");
							} else {
								printf("Released\n");
							}
							return true;
						} });
	}

	// Run the game.
	bool should_run = true;
	while (should_run) {
		// GAME LOGIC
		input_manager->process_input();
		display_manager.poll_events();

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
