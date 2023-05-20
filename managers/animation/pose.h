#ifndef SILENCE_POSE_H
#define SILENCE_POSE_H

#include <glm/gtx/quaternion.hpp>
// Slim transform for animations
struct Xform {
	glm::vec3 translation;
	glm::quat rotation;

	Xform operator*(const Xform &other) const {
		return Xform{ translation + rotation * other.translation, rotation * other.rotation };
	}

	void operator*=(const Xform &other) {
		translation += rotation * other.translation;
		rotation *= other.rotation;
	}

	explicit operator glm::mat4() const {
		return glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation);
	}
};

struct Pose {
	std::vector<Xform> xforms;
};

#endif //SILENCE_POSE_H
