#ifndef SILENCE_COLLIDER_OBB_H
#define SILENCE_COLLIDER_OBB_H

struct ColliderOBB {
	glm::vec3 center;
	glm::vec3 orientation[2];
	glm::vec3 range;

	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["center"] = nlohmann::json::object();
		obj["range"] = nlohmann::json::object();
		obj["orientation"] = nlohmann::json::object();
		obj["orientation"]["0"] = nlohmann::json::object();
		obj["orientation"]["1"] = nlohmann::json::object();
		obj["center"]["x"] = center.x;
		obj["center"]["y"] = center.y;
		obj["center"]["z"] = center.z;
		obj["orientation"]["0"]["x"] = orientation[0].x;
		obj["orientation"]["0"]["y"] = orientation[0].y;
		obj["orientation"]["0"]["z"] = orientation[0].z;
		obj["orientation"]["1"]["x"] = orientation[1].x;
		obj["orientation"]["1"]["y"] = orientation[1].y;
		obj["orientation"]["1"]["z"] = orientation[1].z;
		obj["range"]["x"] = range.x;
		obj["range"]["y"] = range.y;
		obj["range"]["z"] = range.z;
		j.push_back(nlohmann::json::object());
		j.back()["collider_obb"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializer::get_data("collider_aabb", j);
		center.x = obj["center"]["x"];
		center.y = obj["center"]["y"];
		center.z = obj["center"]["z"];
		orientation[0].x = obj["orientation"]["0"]["x"];
		orientation[0].y = obj["orientation"]["0"]["y"];
		orientation[0].z = obj["orientation"]["0"]["z"];
		orientation[1].x = obj["orientation"]["1"]["x"];
		orientation[1].y = obj["orientation"]["1"]["y"];
		orientation[1].z = obj["orientation"]["1"]["z"];
		range.x = obj["range"]["x"];
		range.y = obj["range"]["y"];
		range.z = obj["range"]["z"];
	}

	void setup_range(const std::vector<glm::vec3> &vertices) {
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
