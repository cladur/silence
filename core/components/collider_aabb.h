#ifndef SILENCE_COLLIDER_AABB_H
#define SILENCE_COLLIDER_AABB_H
#include "glm/glm.hpp"

struct ColliderAABB {
	// Center of collider
	glm::vec3 center;
	// Distances between center and faces in 3 directions, something like "cube radius"
	glm::vec3 range;
	bool is_movable;
};

#endif //SILENCE_COLLIDER_AABB_H
