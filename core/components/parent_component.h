#ifndef SILENCE_PARENT_COMPONENT_H
#define SILENCE_PARENT_COMPONENT_H

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
};

#endif //SILENCE_PARENT_COMPONENT_H
