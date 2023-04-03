#ifndef SILENCE_INPUT_KEY_H
#define SILENCE_INPUT_KEY_H

#include "GLFW/glfw3.h"
#include <string>

enum class InputKey {
	UNKNOWN,

	A,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,
	SPACE,
	ENTER,
	ESCAPE,
	BACKSPACE,
	TAB,
	LEFT_CONTROL,
	RIGHT_CONTROL,
	LEFT_SHIFT,
	RIGHT_SHIFT,
	LEFT_ALT,
	RIGHT_ALT,

	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,

	GAMEPAD_LEFT_STICK_X_POSITIVE,
	GAMEPAD_LEFT_STICK_X_NEGATIVE,
	GAMEPAD_LEFT_STICK_Y_POSITIVE,
	GAMEPAD_LEFT_STICK_Y_NEGATIVE,
	GAMEPAD_RIGHT_STICK_X_POSITIVE,
	GAMEPAD_RIGHT_STICK_X_NEGATIVE,
	GAMEPAD_RIGHT_STICK_Y_POSITIVE,
	GAMEPAD_RIGHT_STICK_Y_NEGATIVE,
	GAMEPAD_LEFT_TRIGGER,
	GAMEPAD_RIGHT_TRIGGER,
	GAMEPAD_LEFT_BUMPER,
	GAMEPAD_RIGHT_BUMPER,
	GAMEPAD_BUTTON_Y,
	GAMEPAD_BUTTON_A,
	GAMEPAD_BUTTON_B,
	GAMEPAD_BUTTON_X,
	GAMEPAD_START,
	GAMEPAD_BACK,
	GAMEPAD_LEFT_THUMB,
	GAMEPAD_RIGHT_THUMB,
	GAMEPAD_DPAD_UP,
	GAMEPAD_DPAD_DOWN,
	GAMEPAD_DPAD_LEFT,
	GAMEPAD_DPAD_RIGHT,
	GAMEPAD_GUIDE,

};

static InputKey glfw_mouse_button_to_input_key(int button) {
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

static InputKey glfw_gamepad_button_to_input_key(int button) {
	switch (button) {
		case GLFW_GAMEPAD_BUTTON_A:
			return InputKey::GAMEPAD_BUTTON_A;
		case GLFW_GAMEPAD_BUTTON_B:
			return InputKey::GAMEPAD_BUTTON_B;
		case GLFW_GAMEPAD_BUTTON_X:
			return InputKey::GAMEPAD_BUTTON_X;
		case GLFW_GAMEPAD_BUTTON_Y:
			return InputKey::GAMEPAD_BUTTON_Y;
		case GLFW_GAMEPAD_BUTTON_LEFT_BUMPER:
			return InputKey::GAMEPAD_LEFT_BUMPER;
		case GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER:
			return InputKey::GAMEPAD_RIGHT_BUMPER;
		case GLFW_GAMEPAD_BUTTON_BACK:
			return InputKey::GAMEPAD_BACK;
		case GLFW_GAMEPAD_BUTTON_START:
			return InputKey::GAMEPAD_START;
		case GLFW_GAMEPAD_BUTTON_GUIDE:
			return InputKey::GAMEPAD_GUIDE;
		case GLFW_GAMEPAD_BUTTON_LEFT_THUMB:
			return InputKey::GAMEPAD_LEFT_THUMB;
		case GLFW_GAMEPAD_BUTTON_RIGHT_THUMB:
			return InputKey::GAMEPAD_RIGHT_THUMB;
		case GLFW_GAMEPAD_BUTTON_DPAD_UP:
			return InputKey::GAMEPAD_DPAD_UP;
		case GLFW_GAMEPAD_BUTTON_DPAD_RIGHT:
			return InputKey::GAMEPAD_DPAD_RIGHT;
		case GLFW_GAMEPAD_BUTTON_DPAD_DOWN:
			return InputKey::GAMEPAD_DPAD_DOWN;
		case GLFW_GAMEPAD_BUTTON_DPAD_LEFT:
			return InputKey::GAMEPAD_DPAD_LEFT;
		default:
			return InputKey::UNKNOWN;
	}
}

static InputKey glfw_key_to_input_key(int key) {
	switch (key) {
		case GLFW_KEY_A:
			return InputKey::A;
		case GLFW_KEY_B:
			return InputKey::B;
		case GLFW_KEY_C:
			return InputKey::C;
		case GLFW_KEY_D:
			return InputKey::D;
		case GLFW_KEY_E:
			return InputKey::E;
		case GLFW_KEY_F:
			return InputKey::F;
		case GLFW_KEY_G:
			return InputKey::G;
		case GLFW_KEY_H:
			return InputKey::H;
		case GLFW_KEY_I:
			return InputKey::I;
		case GLFW_KEY_J:
			return InputKey::J;
		case GLFW_KEY_K:
			return InputKey::K;
		case GLFW_KEY_L:
			return InputKey::L;
		case GLFW_KEY_M:
			return InputKey::M;
		case GLFW_KEY_N:
			return InputKey::N;
		case GLFW_KEY_O:
			return InputKey::O;
		case GLFW_KEY_P:
			return InputKey::P;
		case GLFW_KEY_Q:
			return InputKey::Q;
		case GLFW_KEY_R:
			return InputKey::R;
		case GLFW_KEY_S:
			return InputKey::S;
		case GLFW_KEY_T:
			return InputKey::T;
		case GLFW_KEY_U:
			return InputKey::U;
		case GLFW_KEY_V:
			return InputKey::V;
		case GLFW_KEY_W:
			return InputKey::W;
		case GLFW_KEY_X:
			return InputKey::X;
		case GLFW_KEY_Y:
			return InputKey::Y;
		case GLFW_KEY_Z:
			return InputKey::Z;
		case GLFW_KEY_SPACE:
			return InputKey::SPACE;
		case GLFW_KEY_ENTER:
			return InputKey::ENTER;
		case GLFW_KEY_ESCAPE:
			return InputKey::ESCAPE;
		case GLFW_KEY_BACKSPACE:
			return InputKey::BACKSPACE;
		case GLFW_KEY_TAB:
			return InputKey::TAB;
		case GLFW_KEY_LEFT_CONTROL:
			return InputKey::LEFT_CONTROL;
		case GLFW_KEY_RIGHT_CONTROL:
			return InputKey::RIGHT_CONTROL;
		case GLFW_KEY_LEFT_SHIFT:
			return InputKey::LEFT_SHIFT;
		case GLFW_KEY_RIGHT_SHIFT:
			return InputKey::RIGHT_SHIFT;
		case GLFW_KEY_LEFT_ALT:
			return InputKey::LEFT_ALT;
		case GLFW_KEY_RIGHT_ALT:
			return InputKey::RIGHT_ALT;
		default:
			return InputKey::UNKNOWN;
	}
}

#endif //SILENCE_INPUT_KEY_H
