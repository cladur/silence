#ifndef SILENCE_CHILDREN_COMPONENT_H
#define SILENCE_CHILDREN_COMPONENT_H

#include "types.h"
struct Children {
	std::uint8_t children_count{};
	std::array<Entity, MAX_CHILDREN> children{};

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["children_count"] = children_count;
		serialized_component["children"] = nlohmann::json::array();
		for (int i = 0; i < children_count; i++) {
			serialized_component["children"].push_back(children[i]);
		}
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Children";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		children_count = serialized_component["children_count"];
		for (int i = 0; i < children_count; i++) {
			children[i] = serialized_component["children"][i];
		}
	}

	bool add_child(Entity entity) {
		if (children_count >= MAX_CHILDREN || get_children_index(entity) != -1) {
			return false;
		}

		children[children_count] = entity;
		children_count++;
		return true;
	}

	bool remove_child(Entity entity) {
		if (children_count == 0) {
			return false;
		}

		int children_index = get_children_index(entity);

		if (children_index != -1) {
			children[children_index] = children[children_count - 1];
			children[children_count - 1] = 0;
			children_count--;
			return true;
		}

		return false;
	}

	int get_children_index(Entity entity) {
		auto entity_iterator = std::find(children.begin(), children.end(), entity);

		if (entity_iterator != std::end(children)) {
			return entity_iterator - children.begin();
		} else {
			return -1;
		}
	}
};

#endif //SILENCE_CHILDREN_COMPONENT_H
