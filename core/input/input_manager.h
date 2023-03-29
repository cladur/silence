#ifndef SILENCE_INPUT_MANAGER_H
#define SILENCE_INPUT_MANAGER_H

#pragma once
#include "input_devices.h"
#include "input_key.h"
#include "magic_enum.hpp"
#include <spdlog/spdlog.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class InputManager {
public:
	using ActionCallbackFunc = std::function<bool(InputSource, int, float)>;
	struct ActionCallback {
		std::string callback_reference;
		ActionCallbackFunc func;
	};

private:
	struct ActionEvent {
		std::string action_name;
		InputSource source;
		int source_index;
		float value;
	};

	static InputManager *singleton;

	std::unordered_map<InputKey, std::vector<InputAction>> input_action_mapping{};
	std::unordered_map<std::string, std::vector<ActionCallback>> action_callbacks{};

	std::vector<InputDevice> input_devices;

public:
	InputManager();
	~InputManager();

	//process_input will get new device state and compare it to previous state; then generate action events
	void process_input();
	std::vector<ActionEvent> generate_action_event(int DeviceIndex, InputKey key, float newVal);
	void propagate_action_event(ActionEvent event);

	void register_device(const InputDevice &device);
	void remove_device(InputDeviceType type, int input_index);

	void register_action_callback(const std::string &action_name, const ActionCallback &callback);
	void remove_action_callback(const std::string &action_name, const std::string &callback_reference);

	void map_input_to_action(InputKey key, const InputAction &action);
	void unmap_input_from_action(InputKey key, const std::string &action);
};

#endif //SILENCE_INPUT_MANAGER_H
