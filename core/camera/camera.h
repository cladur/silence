#ifndef SILENCE_CAMERA_H
#define SILENCE_CAMERA_H

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 500.0f;
const float SENSITIVITY = 160.0f;
const float FOV = 60.0f;

class Camera {
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;

	// Camera options
	float movement_speed;
	float mouse_sensitivity;
	float zoom;

public:
	float fov;

	//euler angles
	float yaw;
	float pitch;

	explicit Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
			float yaw = 90.0f, float pitch = 0.0f);

	[[nodiscard]] glm::mat4 get_view_matrix() const;

	void move_forward(float dt);
	void move_right(float dt);
	void move_up(float dt);
	void rotate(float x_offset, float y_offset);

private:
	void update_camera_vectors();
};

#endif //SILENCE_CAMERA_H
