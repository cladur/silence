#ifndef SILENCE_ROTATOR_COMPONENT_H
#define SILENCE_ROTATOR_COMPONENT_H

#include <glm/fwd.hpp>
struct Rotator {
	glm::vec3 rotation_speed;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["rotation_speed"] = nlohmann::json::object();
		serialized_component["rotation_speed"]["x"] = rotation_speed.x;
		serialized_component["rotation_speed"]["y"] = rotation_speed.y;
		serialized_component["rotation_speed"]["z"] = rotation_speed.z;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Rotator";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		rotation_speed.x = serialized_component["rotation_speed"]["x"];
		rotation_speed.y = serialized_component["rotation_speed"]["y"];
		rotation_speed.z = serialized_component["rotation_speed"]["z"];
	}
};

#endif //SILENCE_ROTATOR_COMPONENT_H