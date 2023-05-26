#ifndef SILENCE_EXPLODING_BOX_COMPONENT_H
#define SILENCE_EXPLODING_BOX_COMPONENT_H

#include <glm/fwd.hpp>
struct ExplodingBox {
	float explosion_radius = 0.0F;
	float distraction_radius = 0.0F;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["explosion_radius"] = explosion_radius;
		serialized_component["distraction_radius"] = distraction_radius;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ExplodingBox";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		explosion_radius = serialized_component["explosion_radius"];
		distraction_radius = serialized_component["distraction_radius"];
	}
};

#endif //SILENCE_EXPLODING_BOX_COMPONENT_H
