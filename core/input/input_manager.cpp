#include "input_manager.h"
InputSource get_input_source_from_key(InputKey key) {
	switch (key) {
		case InputKey::A:
		case InputKey::B:
		case InputKey::C:
		case InputKey::D:
		case InputKey::E:
		case InputKey::F:
		case InputKey::G:
		case InputKey::H:
		case InputKey::I:
		case InputKey::J:
		case InputKey::K:
		case InputKey::L:
		case InputKey::M:
		case InputKey::N:
		case InputKey::O:
		case InputKey::P:
		case InputKey::Q:
		case InputKey::R:
		case InputKey::S:
		case InputKey::T:
		case InputKey::U:
		case InputKey::V:
		case InputKey::W:
		case InputKey::X:
		case InputKey::Y:
		case InputKey::Z:
		case InputKey::SPACE:
		case InputKey::ENTER:
		case InputKey::ESCAPE:
		case InputKey::BACKSPACE:
		case InputKey::TAB:
		case InputKey::LEFT_CONTROL:
		case InputKey::RIGHT_CONTROL:
		case InputKey::LEFT_SHIFT:
		case InputKey::RIGHT_SHIFT:
		case InputKey::LEFT_ALT:
		case InputKey::RIGHT_ALT:
			return InputSource::KEYBOARD;
		case InputKey::MOUSE_X:
		case InputKey::MOUSE_Y:
		case InputKey::MOUSE_LEFT:
		case InputKey::MOUSE_RIGHT:
		case InputKey::MOUSE_MIDDLE:
			return InputSource::MOUSE;
		case InputKey::L_STICK_X:
		case InputKey::L_STICK_Y:
		case InputKey::R_STICK_X:
		case InputKey::R_STICK_Y:
		case InputKey::L_TRIGGER:
		case InputKey::R_TRIGGER:
		case InputKey::L_BUMPER:
		case InputKey::R_BUMPER:
		case InputKey::GAMEPAD_BUTTON_NORTH:
		case InputKey::GAMEPAD_BUTTON_SOUTH:
		case InputKey::GAMEPAD_BUTTON_EAST:
		case InputKey::GAMEPAD_BUTTON_WEST:
		case InputKey::GAMEPAD_START:
		case InputKey::GAMEPAD_BACK:
		case InputKey::GAMEPAD_LEFT_THUMB:
		case InputKey::GAMEPAD_RIGHT_THUMB:
		case InputKey::GAMEPAD_DPAD_UP:
		case InputKey::GAMEPAD_DPAD_DOWN:
		case InputKey::GAMEPAD_DPAD_LEFT:
		case InputKey::GAMEPAD_DPAD_RIGHT:
		case InputKey::GAMEPAD_GUIDE:
			return InputSource::GAMEPAD;
		default:
			return InputSource::UNKNOWN;
	}
}

void InputManager::register_action_callback(
		const std::string &action_name, const InputManager::ActionCallback &callback) {
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
void InputManager::register_device(const InputDevice &device) {
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
float InputManager::get_action_raw_value(const std::string &action_name) {
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
float InputManager::get_action_value(const std::string &action_name) {
	float value = 0;
	for (auto &device : input_devices) {
		for (auto &key_state : device.CurrentState) {
			auto &actions = input_action_mapping[key_state.first];
			for (auto &action : actions) {
				if (action.action_name == action_name) {
					if(abs(key_state.second.value) > abs(action.deadzone))
					{
						return key_state.second.value * action.scale;
					}
				}
			}
		}
	}
	return value;
}

float InputManager::get_axis(const std::string &negative_action, const std::string &positive_action) {
	float negative_value = get_action_raw_value(negative_action);
	float positive_value = get_action_raw_value(positive_action);
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
float InputManager::get_mouse_x() {
	for (auto &device : input_devices) {
		if (device.Type == InputDeviceType::MOUSE) {
			return device.CurrentState[InputKey::MOUSE_X].value;
		}
	}
	return 0;
}
float InputManager::get_mouse_y() {
	for (auto &device : input_devices) {
		if (device.Type == InputDeviceType::MOUSE) {
			return device.CurrentState[InputKey::MOUSE_Y].value;
		}
	}
	return 0;
}

InputManager::InputManager() = default;
InputManager::~InputManager() = default;
