#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) :
		front(glm::vec3(0.0f, 0.0f, -1.0f)),
		movement_speed(SPEED),
		mouse_sensitivity(SENSITIVITY),
		fov(FOV),
		position(position),
		up(up),
		yaw(yaw),
		pitch(pitch) {
	update_camera_vectors();
}

glm::mat4 Camera::get_view_matrix() const {
	glm::vec3 temp_position = position;
	temp_position.y *= -1.f;
	return glm::lookAt(temp_position, temp_position + front, up);
}

void Camera::move_forward(float dt) {
	glm::vec3 diff = front * dt * movement_speed;
	diff.y *= -1.f;
	position += diff;
}

void Camera::move_right(float dt) {
	position += right * dt * movement_speed;
}

void Camera::move_up(float dt) {
	glm::vec3 diff = up * dt * movement_speed;
	diff.y *= -1.f;
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
