#ifndef SILENCE_COLLISION_SYSTEM_H
#define SILENCE_COLLISION_SYSTEM_H

#include "base_system.h"

struct ColliderSphere;
struct ColliderAABB;
struct ColliderOBB;

enum class CollisionFlag : uint16_t {
	NONE = 0b0000000000000000,
	FIRST_SPHERE = 0b0000000000000001,
	SECOND_SPHERE = 0b0000000000000010,
	FIRST_AABB = 0b0000000000000100,
	SECOND_AABB = 0b0000000000001000,
	FIRST_OBB = 0b0000000000010000,
	SECOND_OBB = 0b0000000000100000,
	SPHERE_SPHERE = FIRST_SPHERE | SECOND_SPHERE,
	AABB_AABB = FIRST_AABB | SECOND_AABB,
	SPHERE_AABB = FIRST_SPHERE | SECOND_AABB,
	AABB_SPHERE = FIRST_AABB | SECOND_SPHERE,
	OBB_OBB = FIRST_OBB | SECOND_OBB,
	SPHERE_OBB = FIRST_SPHERE | SECOND_OBB,
	OBB_SPHERE = FIRST_OBB | SECOND_SPHERE,
	AABB_OBB = FIRST_AABB | SECOND_OBB,
	OBB_AABB = FIRST_OBB | SECOND_AABB,
};

static CollisionFlag operator|(const CollisionFlag first, const CollisionFlag second) {
	return CollisionFlag(uint16_t(first) | uint16_t(second));
}

class CollisionSystem : public BaseSystem {
public:
	void startup();
	void update();

private:
	bool is_overlap(ColliderSphere &a, ColliderSphere &b);
	void resolve_collision_sphere(Entity e1, Entity e2);

	bool is_overlap(ColliderAABB &a, ColliderAABB &b);
	void resolve_collision_aabb(Entity e1, Entity e2);

	glm::vec3 is_overlap(ColliderAABB &aabb, ColliderSphere &sphere);
	void resolve_aabb_sphere(Entity aabb, Entity sphere);

	glm::vec3 is_overlap(ColliderOBB &a, ColliderOBB &b);
	void resolve_collision_obb(Entity e1, Entity e2);

	glm::vec3 is_overlap(ColliderOBB &a, ColliderSphere &b);
	void resolve_obb_sphere(Entity obb, Entity sphere);

	glm::vec3 is_overlap(ColliderOBB &a, ColliderAABB &b);
	void resolve_obb_aabb(Entity obb, Entity aabb);
};

#endif //SILENCE_COLLISION_SYSTEM_H
