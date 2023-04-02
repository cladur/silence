#ifndef SILENCE_INPUT_KEY_H
#define SILENCE_INPUT_KEY_H

#pragma once
#include "GLFW/glfw3.h"
#include "string"

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

	MOUSE_POS_X,
	MOUSE_POS_Y,
	MOUSE_X,
	MOUSE_Y,
	MOUSE_LEFT,
	MOUSE_RIGHT,
	MOUSE_MIDDLE,

	L_STICK_X,
	L_STICK_Y,
	R_STICK_X,
	R_STICK_Y,
	L_TRIGGER,
	R_TRIGGER,
	L_BUMPER,
	R_BUMPER,
	GAMEPAD_BUTTON_NORTH,
	GAMEPAD_BUTTON_SOUTH,
	GAMEPAD_BUTTON_EAST,
	GAMEPAD_BUTTON_WEST,
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

enum class InputSource { KEYBOARD, MOUSE, GAMEPAD, UNKNOWN };

struct InputAction {
	std::string action_name;
	float scale{ 1.f };
	[[maybe_unused]] float deadzone{ 0.1f };
};

static InputKey multiplatform_key_to_input_key(int key) {
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
		case GLFW_KEY_W:
			return InputKey::W;
		case GLFW_KEY_S:
			return InputKey::S;
		case GLFW_KEY_R:
			return InputKey::R;
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

static InputKey multiplatform_button_to_input_key(int button) {
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

static InputSource get_input_source_from_key(InputKey key) {
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

#endif //SILENCE_INPUT_KEY_H
