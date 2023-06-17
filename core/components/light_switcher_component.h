#ifndef SILENCE_LIGHT_SWITCHER_COMPONENT_H
#define SILENCE_LIGHT_SWITCHER_COMPONENT_H

#include <glm/fwd.hpp>
struct LightSwitcher {
	float switch_time = 0.0f;
	float switch_time_variance = 0.0f;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["switch_time"] = switch_time;
		serialized_component["switch_time_variance"] = switch_time_variance;

		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "LightSwitcher";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		switch_time = serialized_component["switch_time"];
		switch_time_variance = serialized_component["switch_time_variance"];
	}
};

#endif //SILENCE_LIGHT_SWITCHER_COMPONENT_H