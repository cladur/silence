#ifndef SILENCE_FRUSTUM_H
#define SILENCE_FRUSTUM_H

#include "components/light_component.h"
class DebugCamera;
class Camera;
class Transform;

struct FrustumPlane {
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 point = glm::vec3(0.0f);

	FrustumPlane() = default;
	FrustumPlane(const glm::vec3 &p, const glm::vec3 &normal) : normal(glm::normalize(normal)), point(p) {
		//distance(glm::dot(normal, p)) {
	}

	[[nodiscard]] float get_distance(const glm::vec3 &p) const {
		return glm::dot(normal, p - point);
	}
};

struct Frustum {
	FrustumPlane top;
	FrustumPlane bottom;

	FrustumPlane right;
	FrustumPlane left;

	FrustumPlane near;
	FrustumPlane far;

	Frustum() = default;

	void create_frustum_from_camera(const DebugCamera &cam, float aspect_ratio);
	void create_frustum_from_camera(const Camera &camera, const Transform &transform, float aspect_ratio);
	void create_frustum_from_light(const Light &light, const Transform &transform, float aspect_ratio);
};

#endif //SILENCE_FRUSTUM_H
