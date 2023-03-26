#ifndef SILENCE_INPUT_DEVICES_H
#define SILENCE_INPUT_DEVICES_H

#endif //SILENCE_INPUT_DEVICES_H
#pragma once

#include "input_key.h"
#include <functional>

enum class InputDeviceType {

	KEYBOARD,
	MOUSE,
	GAMEPAD
};

struct InputDeviceState {
	float value;
};

using InputDeviceStateCallbackFunc = std::function<std::unordered_map<InputKey, InputDeviceState>(int)>;

struct InputDevice {
	InputDeviceType Type;
	int Index;
	std::unordered_map<InputKey, InputDeviceState> CurrentState;
	InputDeviceStateCallbackFunc StateFunc;
};
