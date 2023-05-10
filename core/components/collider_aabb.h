#ifndef SILENCE_COLLIDER_AABB_H
#define SILENCE_COLLIDER_AABB_H

struct ColliderAABB {
	// Center of collider
	glm::vec3 center;
	// Distances between center and faces in 3 directions, something like "cube radius"
	glm::vec3 range;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["center"] = nlohmann::json::object();
		serialized_component["range"] = nlohmann::json::object();
		serialized_component["center"]["x"] = center.x;
		serialized_component["center"]["y"] = center.y;
		serialized_component["center"]["z"] = center.z;
		serialized_component["range"]["x"] = range.x;
		serialized_component["range"]["y"] = range.y;
		serialized_component["range"]["z"] = range.z;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ColliderAABB";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		center.x = serialized_component["center"]["x"];
		center.y = serialized_component["center"]["y"];
		center.z = serialized_component["center"]["z"];
		range.x = serialized_component["range"]["x"];
		range.y = serialized_component["range"]["y"];
		range.z = serialized_component["range"]["z"];
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
