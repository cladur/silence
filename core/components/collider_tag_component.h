#ifndef SILENCE_COLLIDER_TAG_COMPONENT_H
#define SILENCE_COLLIDER_TAG_COMPONENT_H

// This component is intentionally empty because it is only for recognize that entity has collision component
struct ColliderTag {
	std::string layer_name = "default";
	bool is_active = true;
	bool is_climbable = false;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["layer_name"] = layer_name;
		serialized_component["is_active"] = is_active;
		serialized_component["is_climbable"] = is_climbable;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ColliderTag";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		layer_name = serialized_component["layer_name"];
		if (serialized_component.contains("is_active")) {
			is_active = serialized_component["is_active"];
		} else {
			is_active = true;
		}

		if (serialized_component.contains("is_climbable")) {
			is_climbable = serialized_component["is_climbable"];
		}
	}
};

#endif //SILENCE_COLLIDER_TAG_COMPONENT_H
