#ifndef SILENCE_FRUSTUM_H
#define SILENCE_FRUSTUM_H

class Camera;

struct FrustumPlane {
	glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 point = glm::vec3(0.0f);

	FrustumPlane() = default;
	FrustumPlane(const glm::vec3& p, const glm::vec3& normal)
			: normal(glm::normalize(normal)),
			point(p) {
			//distance(glm::dot(normal, p)) {
	}

	float get_distance(const glm::vec3& p) const {
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
	void create_frustum_from_camera(const Camera cam);
};

#endif //SILENCE_FRUSTUM_H
