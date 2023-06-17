#ifndef SILENCE_LIGHT_SWITCHER_COMPONENT_H
#define SILENCE_LIGHT_SWITCHER_COMPONENT_H

#include <glm/fwd.hpp>

struct LightSwitcher {
	float turn_on_time = 0.0f;
	float turn_off_time = 0.0f;
	float turn_on_variance = 0.0f;
	float turn_off_variance = 0.0f;

	float time_to_switch = 0.0f;
	float current_time = 0.0f;
	bool is_waiting = false;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["turn_on_time"] = turn_on_time;
		serialized_component["turn_off_time"] = turn_off_time;
		serialized_component["turn_on_variance"] = turn_on_variance;
		serialized_component["turn_off_variance"] = turn_off_variance;

		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "LightSwitcher";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		turn_on_time = serialized_component["turn_on_time"];
		turn_off_time = serialized_component["turn_off_time"];
		turn_on_variance = serialized_component["turn_on_variance"];
		turn_off_variance = serialized_component["turn_off_variance"];
	}
};

#endif //SILENCE_LIGHT_SWITCHER_COMPONENT_H