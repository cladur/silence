#ifndef SILENCE_TRANSFORM_H
#define SILENCE_TRANSFORM_H

#include "types.h"
#include <spdlog/spdlog.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

struct Transform {
private:
	bool changed = true;
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
	void serialize_json(nlohmann::json &serialized_scene) {
		nlohmann::json::object_t serialized_component;
		serialized_component["position"] = nlohmann::json::object();
		serialized_component["orientation"] = nlohmann::json::object();
		serialized_component["scale"] = nlohmann::json::object();
		serialized_component["position"]["x"] = position.x;
		serialized_component["position"]["y"] = position.y;
		serialized_component["position"]["z"] = position.z;
		serialized_component["orientation"]["x"] = orientation.x;
		serialized_component["orientation"]["y"] = orientation.y;
		serialized_component["orientation"]["z"] = orientation.z;
		serialized_component["orientation"]["w"] = orientation.w;
		serialized_component["scale"]["x"] = scale.x;
		serialized_component["scale"]["y"] = scale.y;
		serialized_component["scale"]["z"] = scale.z;
		serialized_scene.push_back(nlohmann::json::object());
		serialized_scene.back()["component_data"] = serialized_component;
		serialized_scene.back()["component_name"] = "Transform";
	}

	void deserialize_json(nlohmann::json &serialized_component) {
		changed = true;
		position.x = serialized_component["position"]["x"];
		position.y = serialized_component["position"]["y"];
		position.z = serialized_component["position"]["z"];
		orientation.x = serialized_component["orientation"]["x"];
		orientation.y = serialized_component["orientation"]["y"];
		orientation.z = serialized_component["orientation"]["z"];
		orientation.w = serialized_component["orientation"]["w"];
		scale.x = serialized_component["scale"]["x"];
		scale.y = serialized_component["scale"]["y"];
		scale.z = serialized_component["scale"]["z"];
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

	[[nodiscard]] glm::vec3 get_global_forward() const {
		return glm::normalize(glm::vec3(global_model_matrix[2]));
	}
	[[nodiscard]] glm::vec3 get_global_up() const {
		return glm::normalize(glm::vec3(global_model_matrix[1]));
	}
	[[nodiscard]] glm::vec3 get_global_right() const {
		return glm::normalize(glm::vec3(global_model_matrix[0]));
	}

	[[nodiscard]] glm::vec3 get_forward() const {
		return glm::normalize(glm::vec3(local_model_matrix[2]));
	}
	[[nodiscard]] glm::vec3 get_up() const {
		return glm::normalize(glm::vec3(local_model_matrix[1]));
	}
	[[nodiscard]] glm::vec3 get_right() const {
		return glm::normalize(glm::vec3(local_model_matrix[0]));
	}

	[[nodiscard]] glm::mat4 get_local_model_matrix() const {
		return local_model_matrix;
	}
	[[nodiscard]] glm::mat4 get_global_model_matrix() const {
		return global_model_matrix;
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

	[[nodiscard]] glm::vec3 get_global_scale() const {
		glm::vec3 scale;
		scale.x = glm::length(glm::vec3(global_model_matrix[0]));
		scale.y = glm::length(glm::vec3(global_model_matrix[1]));
		scale.z = glm::length(glm::vec3(global_model_matrix[2]));
		return scale;
	}

	[[nodiscard]] bool is_changed_this_frame() const {
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
		this->orientation = glm::normalize(glm::quat(new_euler_rot));
		this->changed = true;
	}
	void add_euler_rot(glm::vec3 add_euler_rot) {
		// Convert the Euler angles to quaternions
		glm::quat yaw = glm::angleAxis(add_euler_rot.y, get_up());
		glm::quat pitch = glm::angleAxis(add_euler_rot.x, get_right());
		glm::quat roll = glm::angleAxis(add_euler_rot.z, get_forward());

		// Combine the rotations in the right-handed order
		glm::quat delta_rotation = yaw * pitch * roll;

		// Apply the rotation to the orientation
		this->orientation = delta_rotation * this->orientation;

		// Normalize the quaternion to prevent accumulation of rounding errors
		this->orientation = glm::normalize(this->orientation);

		this->changed = true;
	}

	void add_global_euler_rot(glm::vec3 add_euler_rot) {
		// Convert the Euler angles to quaternions
		glm::quat yaw = glm::angleAxis(add_euler_rot.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::quat pitch = glm::angleAxis(add_euler_rot.x, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat roll = glm::angleAxis(add_euler_rot.z, glm::vec3(0.0f, 0.0f, 1.0f));

		// Combine the rotations in the right-handed order
		glm::quat delta_rotation = yaw * pitch * roll;

		// Apply the rotation to the orientation
		this->orientation = delta_rotation * this->orientation;

		// Normalize the quaternion to prevent accumulation of rounding errors
		this->orientation = glm::normalize(this->orientation);

		this->changed = true;
	}

	void set_orientation(glm::quat new_orientation) {
		this->orientation = glm::normalize(new_orientation);
		this->changed = true;
	}
	void add_orientation(glm::quat add_orientation) {
		this->orientation *= add_orientation;
		this->orientation = glm::normalize(this->orientation);
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