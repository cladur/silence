#ifndef SILENCE_COLLIDER_AABB_H
#define SILENCE_COLLIDER_AABB_H

struct ColliderAABB {
	// Center of collider
	glm::vec3 center;
	// Distances between center and faces in 3 directions, something like "cube radius"
	glm::vec3 range;
	bool is_movable;

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["center"] = nlohmann::json::object();
		obj["range"] = nlohmann::json::object();
		obj["center"]["x"] = center.x;
		obj["center"]["y"] = center.y;
		obj["center"]["z"] = center.z;
		obj["range"]["x"] = range.x;
		obj["range"]["y"] = range.y;
		obj["range"]["z"] = range.z;
		obj["is_movable"] = is_movable;
		j.push_back(nlohmann::json::object());
		j.back()["collider_aabb"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializaer::get_data("collider_aabb", j);
		center.x = obj["center"]["x"];
		center.y = obj["center"]["y"];
		center.z = obj["center"]["z"];
		range.x = obj["range"]["x"];
		range.y = obj["range"]["y"];
		range.z = obj["range"]["z"];
		is_movable = obj["is_movable"];
	}

	glm::vec3 min() const {
		return { center.x - range.x, center.y - range.y, center.z - range.z };
	}

	glm::vec3 max() const {
		return { center.x + range.x, center.y + range.y, center.z + range.z };
	}
};

#endif //SILENCE_COLLIDER_AABB_H
