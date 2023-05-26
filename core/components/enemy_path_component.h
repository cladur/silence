#ifndef SILENCE_ENEMY_PATH_H
#define SILENCE_ENEMY_PATH_H

struct EnemyPath {
	std::vector<glm::vec3> path;
	std::vector<std::pair<float, bool>> patrol_points;
	int next_position = 0;
	float speed = 1.0f;
	float rotation_speed = 1.0f;
	glm::vec3 rotation_start = glm::vec3(0.0f);
	glm::vec3 rotation_end = glm::vec3(0.0f);
	glm::vec3 prev_position = glm::vec3(0.0f);
	bool is_rotating = false;
	bool first_rotation_frame = false;
	bool is_patrolling = false;
	float patrol_cooldown = 0.0f;

	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_scene.push_back(nlohmann::json::object());

		serialized_component["speed"] = speed;
		serialized_component["rotation_speed"] = rotation_speed;

		int i = 0;
		for (auto &pos : path) {

			serialized_component["path_" + std::to_string(i) + "_x"] = pos.x;
			serialized_component["path_" + std::to_string(i) + "_y"] = pos.y;
			serialized_component["path_" + std::to_string(i) + "_z"] = pos.z;

			serialized_component["patrol_points_" + std::to_string(i)] = patrol_points[i].first;
			serialized_component["patrol_times_" + std::to_string(i)] = patrol_points[i].second;

			i++;
		}

		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "EnemyPath";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		int i = 0;
		speed = serialized_component["speed"];
		rotation_speed = serialized_component["rotation_speed"];
		while (true) {
			std::string pos_x = "path_" + std::to_string(i) + "_x";
			std::string pos_y = "path_" + std::to_string(i) + "_y";
			std::string pos_z = "path_" + std::to_string(i) + "_z";

			std::string is_patrol_point = "patrol_points_" + std::to_string(i);
			std::string patrol_time = "patrol_times_" + std::to_string(i);

			if (serialized_component.contains(pos_x) && serialized_component.contains(pos_y) && serialized_component.contains(pos_z)) {
				path.emplace_back(serialized_component[pos_x], serialized_component[pos_y], serialized_component[pos_z]);
				if (serialized_component.contains(is_patrol_point) && serialized_component.contains(patrol_time)) {
					this->patrol_points.emplace_back(serialized_component[is_patrol_point], serialized_component[patrol_time]);
				}
				else {
					this->patrol_points.emplace_back(0.0f, false);
				}
			}
			else {
				break;
			}

			i++;
		}


	}
};

#endif //SILENCE_ENEMY_PATH_H
