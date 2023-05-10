#ifndef SILENCE_STATIC_TAG_COMPONENT_H
#define SILENCE_STATIC_TAG_COMPONENT_H

// This component is intentionally empty because it is only for recognize that entity has static collision
struct StaticTag {
	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "StaticTag";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
	}
};

#endif //SILENCE_STATIC_TAG_COMPONENT_H
