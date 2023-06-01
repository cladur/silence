#ifndef SILENCE_PATH_NODE_COMPONENT_H
#define SILENCE_PATH_NODE_COMPONENT_H

#include <nlohmann/json.hpp>
struct PathNode {
	bool is_patrol_point = false;
	float patrol_time = 0.0f;
	//uint32_t next_node;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["is_patrol_point"] = is_patrol_point;
		serialized_component["patrol_time"] = patrol_time;
		//serialized_component["next_node"] = next_node;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "PathNode";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		is_patrol_point = serialized_component["is_patrol_point"];
		patrol_time = serialized_component["patrol_time"];
		//next_node = serialized_component["next_node"];
	}
};

#endif //SILENCE_PATH_NODE_COMPONENT_H
