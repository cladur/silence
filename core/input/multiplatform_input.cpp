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
