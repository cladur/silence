#ifndef SILENCE_TRANSFORM_H
#define SILENCE_TRANSFORM_H

#include "types.h"
#include <spdlog/spdlog.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
struct Transform {
private:
	bool changed;
	bool changed_this_frame = false;

	glm::mat4 local_model_matrix = glm::mat4(1.0f);
	glm::mat4 global_model_matrix = glm::mat4(1.0f);

	void update_changed_this_frame() {
		changed_this_frame = false;
		if (changed) {
			changed_this_frame = true;
		}
	}

public:
	glm::vec3 position{};
	glm::quat orientation{};
	glm::vec3 scale{};
	void serialize_json(nlohmann::json &j) {
		nlohmann::json::object_t obj;
		obj["position"] = nlohmann::json::object();
		obj["orientation"] = nlohmann::json::object();
		obj["scale"] = nlohmann::json::object();
		obj["position"]["x"] = position.x;
		obj["position"]["y"] = position.y;
		obj["position"]["z"] = position.z;
		obj["orientation"]["x"] = orientation.x;
		obj["orientation"]["y"] = orientation.y;
		obj["orientation"]["z"] = orientation.z;
		obj["orientation"]["w"] = orientation.w;
		obj["scale"]["x"] = scale.x;
		obj["scale"]["y"] = scale.y;
		obj["scale"]["z"] = scale.z;
		j.push_back(nlohmann::json::object());
		j.back()["transform"] = obj;
	}

	void deserialize_json(nlohmann::json &j) {
		nlohmann::json obj = Serializer::get_data("transform", j);
		SPDLOG_WARN(obj.dump());
		position.x = obj["position"]["x"];
		position.y = obj["position"]["y"];
		position.z = obj["position"]["z"];
		orientation.x = obj["orientation"]["x"];
		orientation.y = obj["orientation"]["y"];
		orientation.z = obj["orientation"]["z"];
		orientation.w = obj["orientation"]["w"];
		scale.x = obj["scale"]["x"];
		scale.y = obj["scale"]["y"];
		scale.z = obj["scale"]["z"];
	}
	//constructor
	Transform(glm::vec3 position, glm::vec3 euler_rot, glm::vec3 scale) {
		this->position = position;
		this->orientation = glm::quat(euler_rot);
		this->scale = scale;
		this->changed = true;
	}

	Transform() {
		this->position = glm::vec3(0.0f, 0.0f, 0.0f);
		this->orientation = glm::quat();
		this->scale = glm::vec3(1.0f, 1.0f, 1.0f);
		this->changed = true;
	}
	// write getters
	[[nodiscard]] glm::vec3 get_position() const {
		return position;
	}
	[[nodiscard]] glm::vec3 get_euler_rot() const {
		return glm::eulerAngles(orientation);
	}
	[[nodiscard]] glm::quat get_orientation() const {
		return orientation;
	}
	[[nodiscard]] glm::vec3 get_scale() const {
		return scale;
	}
	[[nodiscard]] bool is_changed() const {
		return changed;
	}

	[[nodiscard]] glm::vec3 get_global_position() const {
		return glm::vec3(global_model_matrix[3]);
	}
	[[nodiscard]] glm::vec3 get_global_euler_rot() const {
		return glm::eulerAngles(glm::quat_cast(global_model_matrix));
	}
	[[nodiscard]] glm::quat get_global_orientation() const {
		return glm::quat_cast(global_model_matrix);
	}

	[[nodiscard]] bool is_changed_this_frame() {
		//		if (changed_this_frame) {
		//			changed_this_frame = false;
		//			return true;
		//		} else {
		//			return false;
		//		}
		return changed_this_frame;
	}

	void set_changed(bool changed) {
		this->changed = changed;
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
		this->orientation = glm::quat(new_euler_rot);
		this->changed = true;
	}
	void add_euler_rot(glm::vec3 add_euler_rot) {
		this->orientation *= glm::quat(add_euler_rot);
		this->changed = true;
	}
	void set_orientation(glm::quat new_orientation) {
		this->orientation = new_orientation;
		this->changed = true;
	}
	void add_orientation(glm::quat add_orientation) {
		this->orientation *= add_orientation;
		this->changed = true;
	}
	void set_scale(glm::vec3 new_scale) {
		this->scale = new_scale;
		this->changed = true;
	}
	void set_changed_this_frame(bool changed_this_frame) {
		changed_this_frame = changed_this_frame;
	}
	glm::mat4 get_global_model_matrix() {
		//update_global_model_matrix();
		return this->global_model_matrix;
	}

	glm::mat4 get_local_model_matrix() {
		if (!changed) {
			return this->local_model_matrix;
		}

		calculate_local_model_matrix();
		return this->local_model_matrix;
	}

	void update_global_model_matrix(glm::mat4 parent_model) {
		update_changed_this_frame();
		this->global_model_matrix = parent_model * get_local_model_matrix();
	}

	void update_global_model_matrix() {
		update_changed_this_frame();
		this->global_model_matrix = get_local_model_matrix();
	}

	void calculate_local_model_matrix() {
		// translation * rotation * scale (also known as TRS matrix)
		this->local_model_matrix = glm::translate(glm::mat4(1.0f), position) * glm::mat4_cast(orientation) *
				glm::scale(glm::mat4(1.0f), scale);

		changed = false;
	}

	void reparent_to(Transform &new_parent) {
		auto relative_affine = glm::inverse(new_parent.get_global_model_matrix()) * get_global_model_matrix();
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(relative_affine, scale, orientation, position, skew, perspective);
		changed = true;
	}
};

#endif //SILENCE_TRANSFORM_H