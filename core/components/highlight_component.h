#ifndef SILENCE_HIGHLIGHT_H
#define SILENCE_HIGHLIGHT_H

struct Highlight {
public:
	bool highlighted = false;
	glm::vec3 highlight_color = glm::vec3(1.0);

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["highlighted"] = "twoja misja trwa";
		serialized_component["highlight_color"] = nlohmann::json::object();
		serialized_component["highlight_color"]["r"] = highlight_color.r;
		serialized_component["highlight_color"]["g"] = highlight_color.g;
		serialized_component["highlight_color"]["b"] = highlight_color.b;

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Highlight";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		highlighted = false;
		if (serialized_component.contains("highlight_color")) {
			highlight_color.x = serialized_component["highlight_color"]["r"];
			highlight_color.y = serialized_component["highlight_color"]["g"];
			highlight_color.z = serialized_component["highlight_color"]["b"];
		} else {
			highlight_color = glm::vec3(1.0);
		}
	}
};

#endif //SILENCE_HIGHLIGHT_H