#ifndef SILENCE_INPUT_MANAGER_H
#define SILENCE_INPUT_MANAGER_H

#pragma once
#include "input_devices.h"
#include "input_key.h"
#include "magic_enum.hpp"
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>
#include <functional>
#include <glm/glm.hpp>
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

	std::unordered_map<InputKey, std::vector<InputAction>> input_action_mapping{};
	std::unordered_map<std::string, std::vector<ActionCallback>> action_callbacks{};
	std::vector<InputDevice> input_devices;

	std::unordered_map<InputKey, InputDeviceState> keyboard_state{};
	std::unordered_map<InputKey, InputDeviceState> mouse_state{};

public:
	InputManager();
	~InputManager();

	//process_input will get new device state and compare it to previous state; then generate action events
	void process_input();
	std::vector<ActionEvent> generate_action_event(int DeviceIndex, InputKey key, float newVal);
	void propagate_action_event(ActionEvent event);

	void add_device(const InputDevice &device);
	void remove_device(InputDeviceType type, int input_index);

	void add_action_callback(const std::string &action_name, const ActionCallback &callback);
	void remove_action_callback(const std::string &action_name, const std::string &callback_reference);

	void map_input_to_action(InputKey key, const InputAction &action);
	void unmap_input_from_action(InputKey key, const std::string &action);

	std::unordered_map<InputKey, InputDeviceState> get_keyboard_state(int index) {
		return keyboard_state;
	}
	std::unordered_map<InputKey, InputDeviceState> get_mouse_state(int index) {
		return mouse_state;
	}
	std::unordered_map<InputKey, InputDeviceState> get_gamepad_state(const GLFWgamepadstate &state);

	void update_keyboard_state(int key, float value);
	void update_mouse_state(int button, float value);
	void update_mouse_position(GLFWwindow *window);

	// TODO: Implement those
	bool is_action_just_pressed(const std::string &action_name);
	bool is_action_just_released(const std::string &action_name);
	bool is_action_pressed(const std::string &action_name);

	glm::vec2 get_mouse_position();
	glm::vec2 get_mouse_delta();

	float get_action_raw_strength(const std::string &action_name);
	float get_action_strength(const std::string &action_name);
	float get_axis(const std::string &negative_action, const std::string &positive_action);

	float get_gamepad_action_value(int gamepad_index, const std::string &action_name);

	int *get_all_gamepads();
};

#endif //SILENCE_INPUT_MANAGER_H
