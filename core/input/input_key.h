#ifndef SILENCE_INPUT_KEY_H
#define SILENCE_INPUT_KEY_H

#pragma once
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
	[[maybe_unused]] float deadzone {0.1f};
};

InputSource get_input_source_from_key(InputKey key);

#endif //SILENCE_INPUT_KEY_H
