#ifndef SILENCE_MULTIPLATFORM_INPUT_H
#define SILENCE_MULTIPLATFORM_INPUT_H

#include "input_devices.h"
#include "input_key.h"
#include <GLFW/glfw3.h>
#include <unordered_map>

class MultiplatformInput {
private:
	std::unordered_map<InputKey, InputDeviceState> keyboard_state{};
	std::unordered_map<InputKey, InputDeviceState> mouse_state{};

	static InputKey multiplatform_key_to_input_key(int key);
	static InputKey multiplatform_button_to_input_key(int button);

public:
	std::unordered_map<InputKey, InputDeviceState> get_keyboard_state(int index) {
		return keyboard_state;
	}
	std::unordered_map<InputKey, InputDeviceState> get_mouse_state(int index) {
		return mouse_state;
	}
	std::unordered_map<InputKey, InputDeviceState> get_gamepad_state(const GLFWgamepadstate &state);

	void update_keyboard_state(int key, float value);
	void update_mouse_state(int button, float value);
	void update_gamepad_state(const GLFWgamepadstate &state);
};

#endif //SILENCE_MULTIPLATFORM_INPUT_H
