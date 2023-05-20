#ifndef SILENCE_POSE_H
#define SILENCE_POSE_H

#include <glm/gtx/quaternion.hpp>
// Slim transform for animations
struct Xform {
	glm::vec3 translation;
	glm::quat rotation;

	glm::mat4 operator*(const Xform &other) const {
		return glm::mat4(*this) * glm::mat4(other);
	}

	explicit operator glm::mat4() const {
		return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation);
	}
};

struct Pose {
	std::vector<Xform> xfroms;
};

#endif //SILENCE_POSE_H
