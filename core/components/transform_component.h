#ifndef SILENCE_TRANSFORM_H
#define SILENCE_TRANSFORM_H

#include <glm/ext/matrix_transform.hpp>
struct Transform {
private:
	glm::vec3 position{};
	glm::vec3 euler_rot{};
	glm::vec3 scale{};
	bool changed;

	glm::mat4 local_model_matrix = glm::mat4(1.0f);
	glm::mat4 global_model_matrix = glm::mat4(1.0f);

public:
	void serialize_json(nlohmann::json &j) {
		nlohmann::json obj;
		obj["position"] = nlohmann::json::object();
		obj["euler_rot"] = nlohmann::json::object();
		obj["scale"] = nlohmann::json::object();
		obj["position"]["x"] = position.x;
		obj["position"]["y"] = position.y;
		obj["position"]["z"] = position.z;
		obj["euler_rot"]["x"] = euler_rot.x;
		obj["euler_rot"]["y"] = euler_rot.y;
		obj["euler_rot"]["z"] = euler_rot.z;
		obj["scale"]["x"] = scale.x;
		obj["scale"]["y"] = scale.y;
		obj["scale"]["z"] = scale.z;
		j.push_back(nlohmann::json::object());
		j.back()["transform"] = obj;
		SPDLOG_INFO("{}", j.dump());
	}
	//constructor
	Transform(glm::vec3 position, glm::vec3 euler_rot, glm::vec3 scale) {
		this->position = position;
		this->euler_rot = euler_rot;
		this->scale = scale;
		this->changed = true;
	}

	Transform() {
		this->position = glm::vec3(0.0f, 0.0f, 0.0f);
		this->euler_rot = glm::vec3(0.0f, 0.0f, 0.0f);
		this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
		this->changed = true;
	}
	// write getters
	[[nodiscard]] glm::vec3 get_position() const {
		return position;
	}
	[[nodiscard]] glm::vec3 get_euler_rot() const {
		return euler_rot;
	}
	[[nodiscard]] glm::vec3 get_scale() const {
		return scale;
	}
	void set_position(glm::vec3 new_position) {
		this->position = new_position;
		this->changed = true;
	}
	void add_position(glm::vec3 add_position) {
		this->position += add_position;
		this->changed = true;
	}
	void set_euler_rot(glm::vec3 new_euler_rot) {
		this->euler_rot = new_euler_rot;
		this->changed = true;
	}
	void add_euler_rot(glm::vec3 add_euler_rot) {
		this->euler_rot += add_euler_rot;
		this->changed = true;
	}
	void set_scale(glm::vec3 new_scale) {
		this->scale = new_scale;
		this->changed = true;
	}

	glm::mat4 get_global_model_matrix() {
		return this->global_model_matrix;
	}

	glm::mat4 get_local_model_matrix() {
		if (!changed) {
			return this->local_model_matrix;
		}

		calculate_local_model_matrix();
		return this->local_model_matrix;
	}

	void update_global_model_matrix(glm::mat4 parent_model = glm::mat4(1.0f)) {
		if (changed) {
			calculate_local_model_matrix();
		}
		this->global_model_matrix = parent_model * get_local_model_matrix();
	}

	void calculate_local_model_matrix() {
		const glm::mat4 transform_x =
				glm::rotate(glm::mat4(1.0f), glm::radians(euler_rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
		const glm::mat4 transform_y =
				glm::rotate(glm::mat4(1.0f), glm::radians(euler_rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
		const glm::mat4 transform_z =
				glm::rotate(glm::mat4(1.0f), glm::radians(euler_rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

		// Y * X * Z
		const glm::mat4 roation_matrix = transform_y * transform_x * transform_z;

		// translation * rotation * scale (also known as TRS matrix)
		this->local_model_matrix =
				glm::translate(glm::mat4(1.0f), position) * roation_matrix * glm::scale(glm::mat4(1.0f), scale);

		changed = false;
	}
};

#endif //SILENCE_TRANSFORM_H