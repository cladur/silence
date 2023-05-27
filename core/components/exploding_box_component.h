#ifndef SILENCE_EXPLODING_BOX_COMPONENT_H
#define SILENCE_EXPLODING_BOX_COMPONENT_H

#include <glm/fwd.hpp>
struct ExplodingBox {
	float explosion_radius = 0.0f;
	float distraction_radius = 0.0f;
	float distraction_time = 1.0f;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["explosion_radius"] = explosion_radius;
		serialized_component["distraction_radius"] = distraction_radius;
		serialized_component["distraction_time"] = distraction_time;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ExplodingBox";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		explosion_radius = serialized_component["explosion_radius"];
		distraction_radius = serialized_component["distraction_radius"];
		if (serialized_component.contains("distraction_time")) {
			distraction_time = serialized_component["distraction_time"];
		}
		else {
			distraction_time = 1.0f;
		}
	}
};

#endif //SILENCE_EXPLODING_BOX_COMPONENT_H
