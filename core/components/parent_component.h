#ifndef SILENCE_PARENT_COMPONENT_H
#define SILENCE_PARENT_COMPONENT_H

#include "types.h"
struct Parent {
	Entity parent;

	void set_parent(Entity entity) {
		if (entity == parent) {
			return;
		}

		parent = entity;
	}

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["parent"] = parent;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Parent";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		parent = serialized_component["parent"];
	}
};

#endif //SILENCE_PARENT_COMPONENT_H
