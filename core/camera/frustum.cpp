#include "frustum.h"
#include "camera.h"
#include "glm/gtx/string_cast.hpp"

void Frustum::create_frustum_from_camera(const Camera cam) {
	float far = cam.get_far();
	float near = cam.get_near();
	float fov = glm::radians(cam.get_fov());
	float aspect_ratio = cam.get_aspect_ratio();
	glm::vec3 position = cam.get_position();
	glm::vec3 front = cam.get_front();
	glm::vec3 up = cam.get_up();
	glm::vec3 right = cam.get_right();

	float half_v_side = far * tanf(fov / 2.0f);
	float half_h_side = half_v_side * aspect_ratio;
	glm::vec3 front_mult_far = far * front;

	this->near = FrustumPlane(position + near * front, front);
	this->far = FrustumPlane(position + front_mult_far, -front);
	this->right = FrustumPlane(position, glm::cross(front_mult_far - right * half_h_side, up));
	this->left = FrustumPlane(position, glm::cross(up, front_mult_far + right * half_h_side));
	this->top = FrustumPlane(position, glm::cross(right, front_mult_far - up * half_v_side));
	this->bottom = FrustumPlane(position, glm::cross(front_mult_far + up * half_v_side, right));
}