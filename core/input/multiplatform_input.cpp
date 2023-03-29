#include "multiplatform_input.h"

void MultiplatformInput::update_keyboard_state(int key, float value) {
	InputKey i_key = multiplatform_key_to_input_key(key);
	keyboard_state[i_key].value = value;
}
void MultiplatformInput::update_mouse_state(int button, float value) {
	InputKey i_key = multiplatform_button_to_input_key(button);
	mouse_state[i_key].value = value;
}
InputKey MultiplatformInput::multiplatform_key_to_input_key(int key) {
	switch (key) {
		case GLFW_KEY_W:
			return InputKey::W;
		case GLFW_KEY_S:
			return InputKey::S;
		case GLFW_KEY_A:
			return InputKey::A;
		case GLFW_KEY_D:
			return InputKey::D;
		case GLFW_KEY_E:
			return InputKey::E;
		case GLFW_KEY_F:
			return InputKey::F;
		case GLFW_KEY_R:
			return InputKey::R;
		default:
			return InputKey::UNKNOWN;
	}
}
InputKey MultiplatformInput::multiplatform_button_to_input_key(int button) {
	switch (button) {
		case GLFW_MOUSE_BUTTON_LEFT:
			return InputKey::MOUSE_LEFT;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return InputKey::MOUSE_RIGHT;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return InputKey::MOUSE_MIDDLE;
		default:
			return InputKey::UNKNOWN;
	}
}
std::unordered_map<InputKey, InputDeviceState> MultiplatformInput::get_gamepad_state(const GLFWgamepadstate &state) {
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
