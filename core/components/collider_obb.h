#ifndef SILENCE_COLLIDER_OBB_H
#define SILENCE_COLLIDER_OBB_H

struct ColliderOBB {
	glm::vec3 center;
	glm::vec3 orientation[2];
	glm::vec3 range;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["center"] = nlohmann::json::object();
		serialized_component["range"] = nlohmann::json::object();
		serialized_component["orientation"] = nlohmann::json::object();
		serialized_component["orientation"]["0"] = nlohmann::json::object();
		serialized_component["orientation"]["1"] = nlohmann::json::object();
		serialized_component["center"]["x"] = center.x;
		serialized_component["center"]["y"] = center.y;
		serialized_component["center"]["z"] = center.z;
		serialized_component["orientation"]["0"]["x"] = orientation[0].x;
		serialized_component["orientation"]["0"]["y"] = orientation[0].y;
		serialized_component["orientation"]["0"]["z"] = orientation[0].z;
		serialized_component["orientation"]["1"]["x"] = orientation[1].x;
		serialized_component["orientation"]["1"]["y"] = orientation[1].y;
		serialized_component["orientation"]["1"]["z"] = orientation[1].z;
		serialized_component["range"]["x"] = range.x;
		serialized_component["range"]["y"] = range.y;
		serialized_component["range"]["z"] = range.z;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "ColliderOBB";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		center.x = serialized_component["center"]["x"];
		center.y = serialized_component["center"]["y"];
		center.z = serialized_component["center"]["z"];
		orientation[0].x = serialized_component["orientation"]["0"]["x"];
		orientation[0].y = serialized_component["orientation"]["0"]["y"];
		orientation[0].z = serialized_component["orientation"]["0"]["z"];
		orientation[1].x = serialized_component["orientation"]["1"]["x"];
		orientation[1].y = serialized_component["orientation"]["1"]["y"];
		orientation[1].z = serialized_component["orientation"]["1"]["z"];
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

	glm::mat3 get_orientation_matrix() const {
		return { orientation[0], orientation[1], glm::normalize(glm::cross(orientation[0], orientation[1])) };
	}

	glm::vec3 get_orientation_z() const {
		return glm::normalize(glm::cross(orientation[0], orientation[1]));
	}

	void set_orientation(const glm::vec3 &rotation) {
		const glm::mat3 orientation_matrix =
				glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f)) *
						glm::rotate(glm::mat4(1.0f), glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f)) *
						glm::rotate(glm::mat4(1.0f), glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f)));
		orientation[0] = glm::normalize(orientation_matrix[0]);
		orientation[1] = glm::normalize(orientation_matrix[1]);
	};
};

#endif //SILENCE_COLLIDER_OBB_H
