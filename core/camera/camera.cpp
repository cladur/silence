#include "camera.h"

AutoCVarFloat cvar_camera_speed("camera.speed", "Camera Speed", 10, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_camera_sensitivity("camera.sensitivity", "Camera Sensitivity", 50, CVarFlags::EditFloatDrag);

Camera::Camera(glm::vec3 position, glm::vec3 up, float fov, float near, float far, float yaw, float pitch) :
		front(glm::vec3(0.0f, 0.0f, -1.0f)),
		mouse_sensitivity(SENSITIVITY),
		position(position),
		up(up),
		fov(fov),
		near(near),
		far(far),
		yaw(yaw),
		pitch(pitch) {
	update_camera_vectors();
}

glm::mat4 Camera::get_view_matrix() const {
	return glm::lookAt(position, position + front, up);
}

[[nodiscard]] glm::vec3 Camera::get_position() const {
	return position;
}

void Camera::move_forward(float dt) {
	glm::vec3 diff = front * dt * cvar_camera_speed.get();
	position += diff;
}

void Camera::move_right(float dt) {
	position += right * dt * cvar_camera_speed.get();
}

void Camera::move_up(float dt) {
	glm::vec3 diff = glm::vec3(0.0f, 1.0f, 0.0f) * dt * cvar_camera_speed.get();
	position += diff;
}

void Camera::rotate(float x_offset, float y_offset) {
	x_offset *= mouse_sensitivity;
	y_offset *= mouse_sensitivity;

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

void Camera::update_camera_vectors() {
	glm::vec3 new_front;
	new_front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	new_front.y = sin(glm::radians(pitch));
	new_front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(new_front);

	right = glm::normalize(glm::cross(front, glm::vec3(0.0f, 1.0f, 0.0f)));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::set_fov(float fov) {
	this->fov = fov;
}

void Camera::set_render_distance(float near, float far) {
	this->near = near;
	this->far = far;
}

float Camera::get_fov() const {
	return fov;
}

float Camera::get_near() const {
	return near;
}

float Camera::get_far() const {
	return far;
}

void Camera::set_aspect_ratio(float aspect_ratio) {
	this->aspect_ratio = aspect_ratio;
}

float Camera::get_aspect_ratio() const {
	return this->aspect_ratio;
}

glm::vec3 Camera::get_front() const {
	return front;
}

glm::vec3 Camera::get_up() const {
	return up;
}

glm::vec3 Camera::get_right() const {
	return right;
}

Frustum Camera::get_frustum() const {
	return frustum;
}

void Camera::build_frustum() {
	frustum.create_frustum_from_camera(*this);
}
