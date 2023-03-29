#include "input_manager.h"
InputSource get_input_source_from_key(InputKey key) {
	switch (key) {
		case InputKey::W:
		case InputKey::S:
		case InputKey::A:
		case InputKey::D:
		case InputKey::E:
		case InputKey::F:
		case InputKey::R:
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
	//TODO:it will crash when using remove_device() if done like this: for (auto &device : input_devices) {
	//https://stackoverflow.com/questions/8421623/vector-iterators-incompatible + erase_if creates copy of vector
	for (int i = 0; i < input_devices.size(); i++) {
		auto new_state = input_devices[i].StateFunc(input_devices[i].Index);
		//compare to old stare for device
		for (auto &key_state : new_state) {
			if (input_devices[i].CurrentState[key_state.first].value != key_state.second.value) {
				//TODO: Fix conflicting buttons
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
InputManager::InputManager() = default;
InputManager::~InputManager() = default;
