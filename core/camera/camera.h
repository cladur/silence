#ifndef SILENCE_CAMERA_H
#define SILENCE_CAMERA_H

#include "frustum.h"
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SENSITIVITY = 16.0f;

class Camera {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

	float near;
	float far;
	float fov;

	float aspect_ratio;

	// Camera options
	float movement_speed;
	float mouse_sensitivity;
	float zoom;

	Frustum frustum;

public:
	//euler angles
	float yaw;
	float pitch;

	explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
			float fov = 70.0f, float near = 0.1f, float far = 100.0f, float yaw = 90.0f, float pitch = 0.0f);

	[[nodiscard]] glm::mat4 get_view_matrix() const;
	[[nodiscard]] glm::vec3 get_position() const;
	[[nodiscard]] float get_fov() const;
	[[nodiscard]] float get_near() const;
	[[nodiscard]] float get_far() const;
	[[nodiscard]] float get_aspect_ratio() const;
	[[nodiscard]] glm::vec3 get_front() const;
	[[nodiscard]] glm::vec3 get_up() const;
	[[nodiscard]] glm::vec3 get_right() const;
	[[nodiscard]] Frustum get_frustum() const;

	void move_forward(float dt);
	void move_right(float dt);
	void move_up(float dt);
	void rotate(float x_offset, float y_offset);

	void set_fov(float fov);
	void set_render_distance(float near, float far);
	void set_aspect_ratio(float aspect_ratio);
	void build_frustum();

	void update_camera_vectors();
};

#endif //SILENCE_CAMERA_H
