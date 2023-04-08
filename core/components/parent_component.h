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

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["parent"] = parent;
		j.push_back(nlohmann::json::object());
		j.back()["parent"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializaer::get_data("parent", j);
		parent = obj["parent"];
	}
};

#endif //SILENCE_PARENT_COMPONENT_H
