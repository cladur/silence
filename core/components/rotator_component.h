#ifndef SILENCE_ROTATOR_COMPONENT_H
#define SILENCE_ROTATOR_COMPONENT_H

#include <glm/fwd.hpp>
struct Rotator {
	float rotation_x = 0.0f;
	float rotation_y = 0.0f;

	bool is_rotating = true;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["rotation_x"] = rotation_x;
		serialized_component["rotation_y"] = rotation_y;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Rotator";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		rotation_x = serialized_component["rotation_x"];
		rotation_y = serialized_component["rotation_y"];
	}
};

#endif //SILENCE_ROTATOR_COMPONENT_H