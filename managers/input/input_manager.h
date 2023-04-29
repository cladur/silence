#ifndef SILENCE_INPUT_MANAGER_H
#define SILENCE_INPUT_MANAGER_H

#include "input_key.h"

class InputManager {
public:
	struct Action {
		std::string name;
		std::vector<InputKey> keys;
		float deadzone;
	};

private:
	glm::vec2 last_mouse_position{};
	glm::vec2 mouse_position{};
	glm::vec2 mouse_delta{};

	std::unordered_map<std::string, Action> actions;

	std::unordered_map<InputKey, float> previous_key_state{};
	std::unordered_map<InputKey, float> key_state{};

	struct GamepadInfo {
		std::string name;
	};

	std::unordered_map<int, GamepadInfo> gamepads;

	void poll_stick_axis(GLFWgamepadstate &state, int glfw_axis, InputKey positive_key, InputKey negative_key);
	bool is_action_valid(const std::string &action_name);
	void poll_gamepads();

public:
	static InputManager &get();

	void startup();
	void shutdown();

	void add_action(const std::string &action_name);
	void remove_action(const std::string &action_name);
	void add_key_to_action(const std::string &action_name, InputKey key);
	void remove_key_from_action(const std::string &action_name, InputKey key);

	void process_input();

	bool is_action_just_pressed(const std::string &action_name);
	bool is_action_just_released(const std::string &action_name);
	bool is_action_pressed(const std::string &action_name);

	glm::vec2 get_mouse_position();
	glm::vec2 get_mouse_delta();

	float get_action_strength(const std::string &action_name);
	float get_axis(const std::string &negative_action, const std::string &positive_action);
};

#endif //SILENCE_INPUT_MANAGER_H
