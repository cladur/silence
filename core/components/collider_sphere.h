#ifndef SILENCE_COLLIDER_SPHERE_H
#define SILENCE_COLLIDER_SPHERE_H

struct ColliderSphere {
	// Center of collider
	glm::vec3 center;
	float radius;
	bool is_movable;

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["center"] = nlohmann::json::object();
		obj["center"]["x"] = center.x;
		obj["center"]["y"] = center.y;
		obj["center"]["z"] = center.z;
		obj["radius"] = radius;
		obj["is_movable"] = is_movable;
		j.push_back(nlohmann::json::object());
		j.back()["collider_sphere"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializaer::get_data("collider_sphere", j);
		center.x = obj["center"]["x"];
		center.y = obj["center"]["y"];
		center.z = obj["center"]["z"];
		radius = obj["radius"];
		is_movable = obj["is_movable"];
	}
};

#endif //SILENCE_COLLIDER_SPHERE_H
