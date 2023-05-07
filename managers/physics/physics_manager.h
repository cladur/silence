#ifndef SILENCE_PHYSICS_MANAGER_H
#define SILENCE_PHYSICS_MANAGER_H

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

class World;

class PhysicsManager {
public:
	static PhysicsManager &get();

	void resolve_collision(World &world, Entity movable_object, const std::set<Entity> &static_entities);

	bool is_overlap(const ColliderSphere &a, const ColliderSphere &b);
	void resolve_collision_sphere(World &world, Entity e1, Entity e2);

	bool is_overlap(const ColliderAABB &a, const ColliderAABB &b);
	void resolve_collision_aabb(World &world, Entity e1, Entity e2);

	glm::vec3 is_overlap(const ColliderAABB &a, const ColliderSphere &b);
	void resolve_aabb_sphere(World &world, Entity aabb, Entity sphere);

	glm::vec3 is_overlap(const ColliderOBB &a, const ColliderOBB &b);
	void resolve_collision_obb(World &world, Entity e1, Entity e2);

	glm::vec3 is_overlap(const ColliderOBB &a, const ColliderSphere &b);
	void resolve_obb_sphere(World &world, Entity obb, Entity sphere);

	glm::vec3 is_overlap(const ColliderOBB &a, const ColliderAABB &b);
	void resolve_obb_aabb(World &world, Entity obb, Entity aabb);

	bool is_collision_candidate(const glm::vec3 &p1, const glm::vec3 &r1, const glm::vec3 &p2, const glm::vec3 &r2);
};

#endif //SILENCE_PHYSICS_MANAGER_H