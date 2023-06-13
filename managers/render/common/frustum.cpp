#include "frustum.h"

#include "components/camera_component.h"
#include "components/transform_component.h"
#include "debug_camera/debug_camera.h"

void Frustum::create_frustum_from_camera(const DebugCamera &cam, float aspect_ratio) {
	float far = *CVarSystem::get()->get_float_cvar("render.draw_distance.far");
	float near = *CVarSystem::get()->get_float_cvar("render.draw_distance.near");
	float fov = glm::radians(*CVarSystem::get()->get_float_cvar("debug_camera.fov"));
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

void Frustum::create_frustum_from_camera(const Camera &camera, const Transform &transform, float aspect_ratio) {
	float far = *CVarSystem::get()->get_float_cvar("render.draw_distance.far");
	float near = *CVarSystem::get()->get_float_cvar("render.draw_distance.near");
	float fov = glm::radians(camera.fov);
	glm::vec3 position = transform.get_global_position();
	glm::vec3 front = -transform.get_global_forward();
	glm::vec3 up = transform.get_global_up();
	glm::vec3 right = transform.get_global_right();

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

void Frustum::create_frustum_from_light(const Light &light, const Transform &transform, float aspect_ratio) {
	float far = *CVarSystem::get()->get_float_cvar("render.draw_distance.far");
	float near = *CVarSystem::get()->get_float_cvar("render.draw_distance.near");
	float fov = glm::radians(90.0f);
	glm::vec3 position = transform.get_global_position();
	glm::vec3 front = -transform.get_global_forward();
	glm::vec3 up = transform.get_global_up();
	glm::vec3 right = transform.get_global_right();

	float half_v_side = far * tanf(fov * 0.5f);
	float half_h_side = half_v_side * aspect_ratio;
	glm::vec3 front_mult_far = far * front;

	this->near = FrustumPlane(position + near * front, front);
	this->far = FrustumPlane(position + front_mult_far, -front);
	this->right = FrustumPlane(position, glm::cross(front_mult_far - right * half_h_side, up));
	this->left = FrustumPlane(position, glm::cross(up, front_mult_far + right * half_h_side));
	this->top = FrustumPlane(position, glm::cross(right, front_mult_far - up * half_v_side));
	this->bottom = FrustumPlane(position, glm::cross(front_mult_far + up * half_v_side, right));
}
