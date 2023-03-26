#include "input_manager.h"
InputSource GetInputSourceFromKey(InputKey key) {
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
void InputManager::map_input_to_action(InputKey key, const InputAction &action) { //TODO: Check for duplicates
	input_action_mapping[key].emplace_back(action);
}
void InputManager::unmap_input_from_action(InputKey key, const std::string &action) {
	erase_if(input_action_mapping[key],
			[action](const InputAction &input_action) { return input_action.action_name == action; });
}
void InputManager::process_input() {
	std::vector<ActionEvent> events{};
	for (auto &device : input_devices) {
		//get new state for device
		auto new_state = device.StateFunc(device.Index);

		//compare to old stare for device
		for (auto &key_state : new_state) {
			if (device.CurrentState[key_state.first].value != key_state.second.value) {
				//TODO: Fix conflicting buttons
				auto generated_events = generate_action_event(device.Index, key_state.first, key_state.second.value);
				events.insert(events.end(), generated_events.begin(), generated_events.end());

				//save new state value
				device.CurrentState[key_state.first].value = key_state.second.value;
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

	InputSource source = GetInputSourceFromKey(key);
	for (auto &action : actions) {
		action_events.emplace_back(ActionEvent{ .action_name = action.action_name,
				.source = source,
				.source_index = DeviceIndex,
				.value = newVal * action.scale });
	}

	return action_events;
}
void InputManager::propagate_action_event(InputManager::ActionEvent event) {
	for (size_t i = action_callbacks[event.action_name].size() - 1; i >= 0; i--) {
		auto &action_callback = action_callbacks[event.action_name][i];

		if (action_callback.func(event.source, event.source_index, event.value)) {
			break;
		}
	}
}
void InputManager::register_device(const InputDevice &device) {
	input_devices.emplace_back(device);
}
void InputManager::remove_device(InputDeviceType type, int input_index) {
	erase_if(input_devices, [type, input_index](const InputDevice &device) {
		return device.Type == type && device.Index == input_index;
	});
}
InputManager::InputManager() = default;
InputManager::~InputManager() = default;
