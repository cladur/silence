#include "input_manager.h"

void InputManager::add_action_callback(const std::string &action_name, const ActionCallback &callback) {
	action_callbacks[action_name].emplace_back(callback);
}
void InputManager::remove_action_callback(const std::string &action_name, const std::string &callback_reference) {
	std::erase_if(action_callbacks[action_name], [callback_reference](const ActionCallback &callback) {
		return callback.callback_reference == callback_reference;
	});
}
void InputManager::map_input_to_action(InputKey key, const InputAction &action) {
	//TODO: Check for duplicates
	input_action_mapping[key].emplace_back(action);
}
void InputManager::unmap_input_from_action(InputKey key, const std::string &action) {
	erase_if(input_action_mapping[key],
			[action](const InputAction &input_action) { return input_action.action_name == action; });
}
void InputManager::process_input() {
	std::vector<ActionEvent> events{};
	for (int i = 0; i < input_devices.size(); i++) { // NOLINT(modernize-loop-convert)
		auto new_state = input_devices[i].StateFunc(input_devices[i].Index);
		//compare to old stare for device
		for (auto &key_state : new_state) {
			if (input_devices[i].CurrentState[key_state.first].value != key_state.second.value) {
				//TODO: Fix conflicting buttons if they are mapped to the same action
				auto generated_events =
						generate_action_event(input_devices[i].Index, key_state.first, key_state.second.value);
				events.insert(events.end(), generated_events.begin(), generated_events.end());

				//save new state value
				input_devices[i].CurrentState[key_state.first].value = key_state.second.value;
			}
		}
	}

	//propagate action events
	for (auto &event : events) {
		propagate_action_event(event);
	}
}

std::vector<InputManager::ActionEvent> InputManager::generate_action_event(
		int DeviceIndex, InputKey key, float newVal) {
	auto &actions = input_action_mapping[key];

	std::vector<ActionEvent> action_events{};

	InputSource source = get_input_source_from_key(key);
	for (auto &action : actions) {
		action_events.emplace_back(ActionEvent{ .action_name = action.action_name,
				.source = source,
				.source_index = DeviceIndex,
				.value = newVal * action.scale });
	}

	return action_events;
}
void InputManager::propagate_action_event(InputManager::ActionEvent event) {
	size_t var = action_callbacks[event.action_name].size();

	for (int i = var - 1; i >= 0; i--) {
		SPDLOG_INFO(var);
		auto &action_callback = action_callbacks[event.action_name][i];

		if (action_callback.func(event.source, event.source_index, event.value)) {
			break;
		}
	}
}
void InputManager::add_device(const InputDevice &device) {
	input_devices.emplace_back(device);
	SPDLOG_INFO("Register device: {} {}", magic_enum::enum_name(device.Type), device.Index);
	SPDLOG_INFO("No devices: {}", input_devices.size());
}
void InputManager::remove_device(InputDeviceType type, int input_index) {
	erase_if(input_devices, [type, input_index](const InputDevice &device) {
		return device.Type == type && device.Index == input_index;
	});
	SPDLOG_INFO("No devices: {}", input_devices.size());
}
float InputManager::get_action_raw_strength(const std::string &action_name) {
	float value = 0;
	for (auto &device : input_devices) {
		for (auto &key_state : device.CurrentState) {
			auto &actions = input_action_mapping[key_state.first];
			for (auto &action : actions) {
				if (action.action_name == action_name) {
					return key_state.second.value * action.scale;
				}
			}
		}
	}
	return value;
}
float InputManager::get_action_strength(const std::string &action_name) {
	float value = 0;
	for (auto &device : input_devices) {
		for (auto &key_state : device.CurrentState) {
			auto &actions = input_action_mapping[key_state.first];
			for (auto &action : actions) {
				if (action.action_name == action_name) {
					if (abs(key_state.second.value) > abs(action.deadzone)) {
						return key_state.second.value * action.scale;
					}
				}
			}
		}
	}
	return value;
}

float InputManager::get_axis(const std::string &negative_action, const std::string &positive_action) {
	float negative_value = get_action_raw_strength(negative_action);
	float positive_value = get_action_raw_strength(positive_action);
	return negative_value + positive_value;
}
float InputManager::get_gamepad_action_value(int gamepad_index, const std::string &action_name) {
	for (auto &device : input_devices) {
		if (device.Type == InputDeviceType::GAMEPAD && device.Index == gamepad_index) {
			for (auto &key_state : device.CurrentState) {
				auto &actions = input_action_mapping[key_state.first];
				for (auto &action : actions) {
					if (action.action_name == action_name) {
						return key_state.second.value * action.scale;
					}
				}
			}
		}
	}
	return 0;
}
int *InputManager::get_all_gamepads() {
	int *gamepads = new int[input_devices.size()];
	int i = 0;
	for (auto &device : input_devices) {
		if (device.Type == InputDeviceType::GAMEPAD) {
			gamepads[i] = device.Index;
			i++;
		}
	}
	return gamepads;
}

glm::vec2 InputManager::get_mouse_position() {
	for (auto &device : input_devices) {
		if (device.Type == InputDeviceType::MOUSE) {
			return { device.CurrentState[InputKey::MOUSE_POS_X].value,
				device.CurrentState[InputKey::MOUSE_POS_Y].value };
		}
	}
	return glm::vec2(0);
}
glm::vec2 InputManager::get_mouse_delta() {
	for (auto &device : input_devices) {
		if (device.Type == InputDeviceType::MOUSE) {
			return { device.CurrentState[InputKey::MOUSE_X].value, device.CurrentState[InputKey::MOUSE_Y].value };
		}
	}
	return glm::vec2(0);
}
bool InputManager::is_action_pressed(const std::string &action_name) {
	for (auto &device : input_devices) {
		for (auto &key_state : device.CurrentState) {
			auto &actions = input_action_mapping[key_state.first];
			for (auto &action : actions) {
				if (action.action_name == action_name) {
					return key_state.second.value > 0;
				}
			}
		}
	}
	return false;
}
bool InputManager::is_action_just_pressed(const std::string &action_name) {
	return false;
}
bool InputManager::is_action_just_released(const std::string &action_name) {
	return false;
}

void InputManager::update_keyboard_state(int key, float value) {
	InputKey i_key = multiplatform_key_to_input_key(key);
	keyboard_state[i_key].value = value;
}
void InputManager::update_mouse_state(int button, float value) {
	InputKey i_key = multiplatform_button_to_input_key(button);
	mouse_state[i_key].value = value;
}

std::unordered_map<InputKey, InputDeviceState> InputManager::get_gamepad_state(const GLFWgamepadstate &state) {
	std::unordered_map<InputKey, InputDeviceState> gamepad_state{};

	for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; i++) {
		int button_state = state.buttons[i];
		float value = button_state == GLFW_PRESS ? 1.f : 0.f;

		switch (i) {
			case GLFW_GAMEPAD_BUTTON_B:
				gamepad_state[InputKey::GAMEPAD_BUTTON_EAST].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_A:
				gamepad_state[InputKey::GAMEPAD_BUTTON_SOUTH].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_X:
				gamepad_state[InputKey::GAMEPAD_BUTTON_WEST].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_Y:
				gamepad_state[InputKey::GAMEPAD_BUTTON_NORTH].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER:
				gamepad_state[InputKey::L_BUMPER].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER:
				gamepad_state[InputKey::R_BUMPER].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_BACK:
				gamepad_state[InputKey::GAMEPAD_BACK].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_START:
				gamepad_state[InputKey::GAMEPAD_START].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_LEFT_THUMB:
				gamepad_state[InputKey::GAMEPAD_LEFT_THUMB].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB:
				gamepad_state[InputKey::GAMEPAD_RIGHT_THUMB].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_DPAD_UP:
				gamepad_state[InputKey::GAMEPAD_DPAD_UP].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
				gamepad_state[InputKey::GAMEPAD_DPAD_RIGHT].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
				gamepad_state[InputKey::GAMEPAD_DPAD_DOWN].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
				gamepad_state[InputKey::GAMEPAD_DPAD_LEFT].value = value;
				break;
			case GLFW_GAMEPAD_BUTTON_GUIDE:
				gamepad_state[InputKey::GAMEPAD_GUIDE].value = value;
			default:
				break;
		}
	}

	for (int i = 0; i <= GLFW_GAMEPAD_AXIS_LAST; i++) {
		float value = state.axes[i];

		switch (i) {
			case GLFW_GAMEPAD_AXIS_LEFT_X:
				gamepad_state[InputKey::L_STICK_X].value = value;
				break;
			case GLFW_GAMEPAD_AXIS_LEFT_Y:
				gamepad_state[InputKey::L_STICK_Y].value = -value;
				break;
			case GLFW_GAMEPAD_AXIS_RIGHT_X:
				gamepad_state[InputKey::R_STICK_X].value = value;
				break;
			case GLFW_GAMEPAD_AXIS_RIGHT_Y:
				gamepad_state[InputKey::R_STICK_Y].value = -value;
				break;
			case GLFW_GAMEPAD_AXIS_LEFT_TRIGGER:
				gamepad_state[InputKey::L_TRIGGER].value = value;
				break;
			case GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER:
				gamepad_state[InputKey::R_TRIGGER].value = value;
				break;
			default:
				break;
		}
	}

	return gamepad_state;
}
void InputManager::update_mouse_position(GLFWwindow *window) {
	// Set movement
	float last_x = mouse_state[InputKey::MOUSE_POS_X].value;
	float last_y = mouse_state[InputKey::MOUSE_POS_Y].value;

	double x, y;
	glfwGetCursorPos(window, &x, &y);

	mouse_state[InputKey::MOUSE_X].value = static_cast<float>(x) - last_x;
	mouse_state[InputKey::MOUSE_Y].value = static_cast<float>(y) - last_y;
	mouse_state[InputKey::MOUSE_POS_X].value = static_cast<float>(x);
	mouse_state[InputKey::MOUSE_POS_Y].value = static_cast<float>(y);
}

InputManager::InputManager() = default;
InputManager::~InputManager() = default;