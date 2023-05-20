#include "input_manager.h"

#include "display/display_manager.h"
#include <GLFW/glfw3.h>

InputManager &InputManager::get() {
	static InputManager instance;
	return instance;
}

bool InputManager::is_action_valid(const std::string &action_name) {
	if (!actions.contains(action_name)) {
		SPDLOG_WARN("Action {} does not exist", action_name);
		return false;
	}

	auto action = actions[action_name];
	if (action.keys.empty()) {
		// SPDLOG_WARN("Action {} has no keys assigned to it", action_name);
		return false;
	}
	return true;
}

inline void InputManager::poll_stick_axis(
		GLFWgamepadstate &state, int glfw_axis, InputKey positive_key, InputKey negative_key) {
	float value = state.axes[glfw_axis];
	if (value > 0.0f) {
		key_state[positive_key] = value;
		key_state[negative_key] = 0.0f;
	} else {
		key_state[positive_key] = 0.0f;
		key_state[negative_key] = -value;
	}

	float deadzone = 0.25f;
	if (key_state[positive_key] > deadzone) {
		key_state[positive_key] = (key_state[positive_key] - deadzone) / (1.0f - deadzone);
	} else {
		key_state[positive_key] = 0.0f;
	}

	if (key_state[negative_key] > deadzone) {
		key_state[negative_key] = (key_state[negative_key] - deadzone) / (1.0f - deadzone);
	} else {
		key_state[negative_key] = 0.0f;
	}
}

void InputManager::poll_gamepads() {
	GLFWgamepadstate state;

	for (int gamepad = GLFW_JOYSTICK_1; gamepad <= GLFW_JOYSTICK_LAST; gamepad++) {
		if (glfwGetGamepadState(gamepad, &state)) {
			// Buttons
			for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) {
				InputKey key = glfw_gamepad_button_to_input_key(i);
				if (state.buttons[i] == GLFW_PRESS) {
					key_state[key] = 1.0f;
				} else {
					key_state[key] = 0.0f;
				}
			}

			// Sticks
			poll_stick_axis(state, GLFW_GAMEPAD_AXIS_LEFT_X, InputKey::GAMEPAD_LEFT_STICK_X_POSITIVE,
					InputKey::GAMEPAD_LEFT_STICK_X_NEGATIVE);
			poll_stick_axis(state, GLFW_GAMEPAD_AXIS_LEFT_Y, InputKey::GAMEPAD_LEFT_STICK_Y_NEGATIVE,
					InputKey::GAMEPAD_LEFT_STICK_Y_POSITIVE);
			poll_stick_axis(state, GLFW_GAMEPAD_AXIS_RIGHT_X, InputKey::GAMEPAD_RIGHT_STICK_X_POSITIVE,
					InputKey::GAMEPAD_RIGHT_STICK_X_NEGATIVE);
			poll_stick_axis(state, GLFW_GAMEPAD_AXIS_RIGHT_Y, InputKey::GAMEPAD_RIGHT_STICK_Y_NEGATIVE,
					InputKey::GAMEPAD_RIGHT_STICK_Y_POSITIVE);

			// Triggers
			key_state[InputKey::GAMEPAD_LEFT_TRIGGER] = (state.axes[GLFW_GAMEPAD_AXIS_LEFT_TRIGGER] + 1.0f) * 0.5f;
			key_state[InputKey::GAMEPAD_RIGHT_TRIGGER] = (state.axes[GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER] + 1.0f) * 0.5f;
		}
	}
}

void InputManager::startup() {
	GLFWwindow *window = DisplayManager::get().window;

	glfwSetWindowUserPointer(window, this);
	for (int i = GLFW_JOYSTICK_1; i < GLFW_JOYSTICK_16; i++) {
		glfwSetJoystickUserPointer(i, this);
	}

	glfwSetJoystickCallback([](int joystick_id, int event) {
		auto *input = static_cast<InputManager *>(glfwGetJoystickUserPointer(joystick_id));
		if (input == nullptr) {
			return;
		}
		switch (event) {
			case GLFW_CONNECTED: {
				const char *name = glfwGetJoystickName(joystick_id);
				SPDLOG_INFO("Gamepad ({}) {} connected", joystick_id, name);
				break;
			}
			case GLFW_DISCONNECTED: {
				SPDLOG_INFO("Gamepad ({}) disconnected", joystick_id);
				break;
			}
			default:
				break;
		}
	});

	glfwSetKeyCallback(window, [](GLFWwindow *w_window, int key, int scancode, int action, int mods) {
		auto *input = static_cast<InputManager *>(glfwGetWindowUserPointer(w_window));
		InputKey input_key = glfw_key_to_input_key(key);

		if (mods & GLFW_MOD_CONTROL) {
			input->key_state[InputKey::LEFT_CONTROL] = 1.f;
		} else {
			input->key_state[InputKey::LEFT_CONTROL] = 0.f;
		}

		float value = 0.f;
		switch (action) {
			case GLFW_PRESS:
				value = 1.f;
				break;
			case GLFW_RELEASE:
				value = 0.f;
				break;
			case GLFW_REPEAT:
			default:
				return;
		}
		input->key_state[input_key] = value;
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow *w_window, int button, int action, int mods) {
		auto *input = static_cast<InputManager *>(glfwGetWindowUserPointer(w_window));
		InputKey input_key = glfw_mouse_button_to_input_key(button);
		input->key_state[input_key] = action == GLFW_PRESS ? 1.f : 0.f;
	});

	// TODO: Change the y to match OpenGL now that we're not using Vulkan
	glfwSetCursorPosCallback(window, [](GLFWwindow *w_window, double xpos, double ypos) {
		auto *input = static_cast<InputManager *>(glfwGetWindowUserPointer(w_window));
		input->mouse_position = glm::vec2(xpos, ypos);
	});
}

void InputManager::shutdown() {
}

void InputManager::add_action(const std::string &action_name) {
	if (actions.contains(action_name)) {
		SPDLOG_WARN("Action {} already exists", action_name);
		return;
	}
	actions[action_name] = Action{ .name = action_name, .keys = {}, .deadzone = 0.5f };
}

void InputManager::remove_action(const std::string &action_name) {
	actions.erase(action_name);
}

void InputManager::add_key_to_action(const std::string &action_name, InputKey key) {
	if (!actions.contains(action_name)) {
		SPDLOG_WARN("Action {} does not exist", action_name);
	}
	actions[action_name].keys.push_back(key);
}

void InputManager::remove_key_from_action(const std::string &action_name, InputKey key) {
	actions[action_name].keys.erase(
			std::remove(actions[action_name].keys.begin(), actions[action_name].keys.end(), key),
			actions[action_name].keys.end());
}

void InputManager::process_input() {
	ZoneScopedNC("InputManager::process_input", 0x87CEFA);
	previous_key_state = key_state;
	mouse_delta = mouse_position - last_mouse_position;
	last_mouse_position = mouse_position;
	poll_gamepads();
}

bool InputManager::is_action_just_pressed(const std::string &action_name) {
	if (!is_action_valid(action_name)) {
		return false;
	}
	auto action = actions[action_name];

	for (auto &key : action.keys) { // NOLINT(readability-use-anyofallof)
		if (key == InputKey::UNKNOWN) {
			continue;
		}
		if (key_state[key] > action.deadzone && previous_key_state[key] < action.deadzone) {
			return true;
		}
	}
	return false;
}
bool InputManager::is_action_just_released(const std::string &action_name) {
	if (!is_action_valid(action_name)) {
		return false;
	}
	auto action = actions[action_name];

	for (auto &key : action.keys) { // NOLINT(readability-use-anyofallof)
		if (key == InputKey::UNKNOWN) {
			continue;
		}
		if (key_state[key] < action.deadzone && previous_key_state[key] > action.deadzone) {
			return true;
		}
	}
	return false;
}
bool InputManager::is_action_pressed(const std::string &action_name) {
	if (!is_action_valid(action_name)) {
		return false;
	}
	auto action = actions[action_name];

	for (auto &key : action.keys) { // NOLINT(readability-use-anyofallof)
		if (key == InputKey::UNKNOWN) {
			continue;
		}
		if (key_state[key] > action.deadzone) {
			return true;
		}
	}
	return false;
}
glm::vec2 InputManager::get_mouse_position() {
	return mouse_position;
}
glm::vec2 InputManager::get_mouse_delta() {
	return mouse_delta;
}
float InputManager::get_action_strength(const std::string &action_name) {
	if (!is_action_valid(action_name)) {
		return 0.f;
	}

	auto action = actions[action_name];
	float strength = 0.f;
	for (auto &key : action.keys) { // NOLINT(readability-use-anyofallof)
		if (key == InputKey::UNKNOWN) {
			continue;
		}
		if (key_state[key] > strength) {
			strength = key_state[key];
		}
	}

	return strength;
}

float InputManager::get_axis(const std::string &negative_action, const std::string &positive_action) {
	float negative = get_action_strength(negative_action);
	float positive = get_action_strength(positive_action);
	return positive - negative;
}