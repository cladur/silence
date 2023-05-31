#ifndef SILENCE_PATH_PARENT_COMPONENT_H
#define SILENCE_PATH_PARENT_COMPONENT_H

#include <nlohmann/json.hpp>
struct PathParent {

	void serialize_json(nlohmann::json &serialized_scene) {
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = {};
		serialized_scene.back()["component_name"] = "PathParent";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
	}
};

#endif //SILENCE_PATH_PARENT_COMPONENT_H
