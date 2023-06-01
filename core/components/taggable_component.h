#ifndef SILENCE_TAGGABLE_COMPONENT_H
#define SILENCE_TAGGABLE_COMPONENT_H

#include <nlohmann/json.hpp>

struct Taggable {
	glm::vec3 tag_position;
	bool tagged = false;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["tagged"] = tagged;
		serialized_component["tag_position"] = nlohmann::json::object();
		serialized_component["tag_position"]["x"] = tag_position.x;
		serialized_component["tag_position"]["y"] = tag_position.y;
		serialized_component["tag_position"]["z"] = tag_position.z;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Taggable";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		tagged = serialized_component["tagged"];
		tag_position.x = serialized_component["tag_position"]["x"];
		tag_position.y = serialized_component["tag_position"]["y"];
		tag_position.z = serialized_component["tag_position"]["z"];
	}
};

#endif //SILENCE_TAGGABLE_COMPONENT_H
