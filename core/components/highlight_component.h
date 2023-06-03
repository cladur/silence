#ifndef SILENCE_HIGHLIGHT_H
#define SILENCE_HIGHLIGHT_H

struct Highlight {
public:
	bool highlighted = false;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["highlighted"] = "twoja misja trwa";

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Highlight";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		highlighted = false;
	}
};

#endif //SILENCE_HIGHLIGHT_H