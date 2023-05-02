#ifndef SILENCE_COLLIDER_AABB_H
#define SILENCE_COLLIDER_AABB_H

struct ColliderAABB {
	// Center of collider
	glm::vec3 center;
	// Distances between center and faces in 3 directions, something like "cube radius"
	glm::vec3 range;

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
		j.push_back(nlohmann::json::object());
		j.back()["collider_aabb"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializer::get_data("collider_aabb", j);
		center.x = obj["center"]["x"];
		center.y = obj["center"]["y"];
		center.z = obj["center"]["z"];
		range.x = obj["range"]["x"];
		range.y = obj["range"]["y"];
		range.z = obj["range"]["z"];
	}

	void setup_collider(const std::vector<glm::vec3> &vertices) {
		glm::vec3 min(std::numeric_limits<float>::max());
		glm::vec3 max(-std::numeric_limits<float>::max());

		for (const glm::vec3 vertex : vertices) {
			for (int32_t i = 0; i < 3; ++i) {
				if (min[i] > vertex[i]) {
					min[i] = vertex[i];
				}

				if (max[i] < vertex[i]) {
					max[i] = vertex[i];
				}
			}
		}
		range = (max - min) * 0.5f;
	}

	glm::vec3 min() const {
		return center - range;
	}

	glm::vec3 max() const {
		return center + range;
	}
};

#endif //SILENCE_COLLIDER_AABB_H
