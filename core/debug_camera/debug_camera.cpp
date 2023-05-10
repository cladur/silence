#include "debug_camera.h"

#include "components/transform_component.h"

AutoCVarFloat cvar_camera_speed("debug_camera.speed", "Debug Camera Speed", 5, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_camera_fov("debug_camera.fov", "Debug Fov", 70, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_camera_sensitivity(
		"debug_camera.sensitivity", "Debug Camera Sensitivity", 16, CVarFlags::EditFloatDrag);

DebugCamera::DebugCamera(glm::vec3 position, glm::vec3 up, float near, float far, float yaw, float pitch) :
		front(glm::vec3(0.0f, 0.0f, -1.0f)), position(position), up(up), yaw(yaw), pitch(pitch) {
	update_camera_vectors();
}

glm::mat4 DebugCamera::get_view_matrix() const {
	return glm::lookAt(position, position + front, up);
}

[[nodiscard]] glm::vec3 DebugCamera::get_position() const {
	return position;
}

void DebugCamera::move_forward(float dt) {
	glm::vec3 diff = front * dt * cvar_camera_speed.get();
	position += diff;
}

void DebugCamera::move_right(float dt) {
	position += right * dt * cvar_camera_speed.get();
}

void DebugCamera::move_up(float dt) {
	glm::vec3 diff = glm::vec3(0.0f, 1.0f, 0.0f) * dt * cvar_camera_speed.get();
	position += diff;
}

void DebugCamera::rotate(float x_offset, float y_offset) {
	x_offset *= cvar_camera_sensitivity.get();
	y_offset *= cvar_camera_sensitivity.get();

	yaw += x_offset;
	pitch -= y_offset;

	if (pitch > 89.0f) {
		pitch = 89.0f;
	}
	if (pitch < -89.0f) {
		pitch = -89.0f;
	}

	update_camera_vectors();
}

void DebugCamera::update_camera_vectors() {
	glm::vec3 new_front;
	new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	new_front.y = sin(glm::radians(pitch));
	new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(new_front);

	right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, front));
}

void DebugCamera::set_transform(const Transform &transform) {
	this->position = transform.get_global_position();
	this->yaw = transform.get_global_euler_rot().y - 90.0f;
	this->pitch = transform.get_global_euler_rot().x;
	this->update_camera_vectors();
}

glm::vec3 DebugCamera::get_front() const {
	return front;
}

glm::vec3 DebugCamera::get_up() const {
	return up;
}

glm::vec3 DebugCamera::get_right() const {
	return right;
}