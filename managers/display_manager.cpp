#include "display_manager.h"

#include <cassert>

extern DisplayManager display_manager;
extern InputManager *input_manager;

DisplayManager::Status DisplayManager::startup() {
	if (!glfwInit()) {
		return Status::FailedToInitializeGlfw;
	}

	// No need to create context since we're using Vulkan, not OpenGL(ES).
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	window = glfwCreateWindow(640, 480, "Silence Game", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return Status::FailedToCreateWindow;
	}

	if (!glfwVulkanSupported()) {
		glfwTerminate();
		return Status::VulkanNotSupported;
	}
	return Status::Ok;
}

void DisplayManager::setup_input() {
	//input
	glfwSetWindowUserPointer(window, &m_input);

	//registering devices
	for (int i = 0; i <= GLFW_JOYSTICK_LAST; i++) {
		if (glfwJoystickPresent(i)) {
			// Register connected devices
			if (input_manager) {
				//input.AddController(i);
				input_manager->register_device(InputDevice{ .Type = InputDeviceType::GAMEPAD,
						.Index = i,
						.StateFunc = std::bind(
								&DisplayManager::get_gamepad_state, display_manager, std::placeholders::_1) });
			}
		}
	}
	if (input_manager) {
		input_manager->register_device(InputDevice{

				.Type = InputDeviceType::MOUSE,
				.Index = 0,
				.StateFunc = std::bind(&MultiplatformInput::get_mouse_state, &m_input, std::placeholders::_1) });

		input_manager->register_device(InputDevice{

				.Type = InputDeviceType::KEYBOARD,
				.Index = 0,
				.StateFunc = std::bind(&MultiplatformInput::get_keyboard_state, &m_input, std::placeholders::_1) });

		//Setting-up callbacks

		glfwSetJoystickCallback([](int joystickId, int event) {
			if (input_manager) {
				auto *input = static_cast<MultiplatformInput *>(glfwGetWindowUserPointer(display_manager.window));
				if (input) {
					if (event == GLFW_CONNECTED && glfwJoystickIsGamepad(joystickId)) {
						input_manager->register_device(InputDevice{ .Type = InputDeviceType::GAMEPAD,
								.Index = joystickId,
								.StateFunc = std::bind(
										&DisplayManager::get_gamepad_state, display_manager, std::placeholders::_1) });
					} else if (event == GLFW_DISCONNECTED) {
						// The joystick was disconnected
						input_manager->remove_device(InputDeviceType::GAMEPAD, joystickId);
					}
				}
			}
		});

		glfwSetKeyCallback(window, [](GLFWwindow *w_window, int key, int scancode, int action, int mods) {
			auto *input = static_cast<MultiplatformInput *>(glfwGetWindowUserPointer(w_window));
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

		glfwSetMouseButtonCallback(window, [](GLFWwindow *w_window, int button, int action, int mods) {
			auto *input = static_cast<MultiplatformInput *>(glfwGetWindowUserPointer(w_window));
			if (input) {
				input->update_mouse_state(button, action == GLFW_PRESS ? 1.f : 0.f);
			}
		});
	}
}

void DisplayManager::shutdown() {
}

VkSurfaceKHR DisplayManager::create_surface(VkInstance &instance) const {
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface(instance, window, NULL, &surface);
	if (err) {
		assert(false);
	}
	return surface;
}

void DisplayManager::poll_events() const {
	glfwPollEvents();
}

bool DisplayManager::window_should_close() const {
	assert(window != nullptr);
	return glfwWindowShouldClose(window);
}

std::pair<int, int> DisplayManager::get_window_size() const {
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	return std::make_pair(width, height);
}
std::unordered_map<InputKey, InputDeviceState> DisplayManager::get_gamepad_state(int index) {
	GLFWgamepadstate state;
	if (glfwGetGamepadState(index, &state)) {
		return m_input.get_gamepad_state(state);
	}

	return std::unordered_map<InputKey, InputDeviceState>{};
}
