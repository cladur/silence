#ifndef SILENCE_INPUT_KEY_H
#define SILENCE_INPUT_KEY_H

#pragma once
#include "string"

enum class InputKey {
	UNKNOWN,

	W,
	S,
	A,
	D,
	E,
	F,
	R,

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

	//write all the buttons in style ""GAMEPAD_BUTTON_"" + button name

};

enum class InputSource { KEYBOARD, MOUSE, GAMEPAD, UNKNOWN };

struct InputAction {
	std::string action_name;
	float scale{ 1.f };
};

InputSource get_input_source_from_key(InputKey key);

#endif //SILENCE_INPUT_KEY_H
