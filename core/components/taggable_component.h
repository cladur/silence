#ifndef SILENCE_TAGGABLE_COMPONENT_H
#define SILENCE_TAGGABLE_COMPONENT_H

#include <nlohmann/json.hpp>

struct Taggable {
	glm::vec3 tag_position;
	bool tagging = false;
	bool tagged = false;
	float tag_timer = 0.0f;
	float time_to_tag = 1.0f;
	bool fist_frame = true;
	bool enabled = true;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["tag_position"] = nlohmann::json::object();
		serialized_component["tag_position"]["x"] = tag_position.x;
		serialized_component["tag_position"]["y"] = tag_position.y;
		serialized_component["tag_position"]["z"] = tag_position.z;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Taggable";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		tag_position.x = serialized_component["tag_position"]["x"];
		tag_position.y = serialized_component["tag_position"]["y"];
		tag_position.z = serialized_component["tag_position"]["z"];
	}
};

#endif //SILENCE_TAGGABLE_COMPONENT_H
