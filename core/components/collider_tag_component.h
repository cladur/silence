#ifndef SILENCE_COLLIDER_TAG_COMPONENT_H
#define SILENCE_COLLIDER_TAG_COMPONENT_H

// This component is intentionally empty because it is only for recognize that entity has collision component
struct ColliderTag {
	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ColliderTag";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		
	}
};

#endif //SILENCE_COLLIDER_TAG_COMPONENT_H
