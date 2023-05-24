#ifndef SILENCE_PHYSICS_MANAGER_H
#define SILENCE_PHYSICS_MANAGER_H

class World;
struct ColliderSphere;
struct ColliderCapsule;
struct ColliderAABB;
struct ColliderOBB;
struct Transform;
struct RigidBody;
struct ColliderTag;

struct Ray {
	glm::vec3 origin;
	glm::vec3 direction;
	std::string layer_name = "default";
	std::vector<Entity> ignore_list;
};

struct HitInfo {
	glm::vec3 point;
	glm::vec3 normal;
	float distance;
	Entity entity = 0;

	HitInfo() : point(0.0f), normal(0.0f), distance(std::numeric_limits<float>::max()) {
	}
};

enum class Side { FRONT, BACK, INTERSECT };

struct Plane {
	glm::vec3 point;
	glm::vec3 normal;
};

struct BSPNode {
	std::shared_ptr<BSPNode> back, front;
	Plane plane;
	std::set<Entity> entities;
};

enum class CollisionFlag : uint16_t {
	NONE = 0b0000000000000000,
	FIRST_SPHERE = 0b0000000000000001,
	SECOND_SPHERE = 0b0000000000000010,
	FIRST_AABB = 0b0000000000000100,
	SECOND_AABB = 0b0000000000001000,
	FIRST_OBB = 0b0000000000010000,
	SECOND_OBB = 0b0000000000100000,
	FIRST_CAPSULE = 0b0000000001000000,
	SECOND_CAPSULE = 0b0000000010000000,
	SPHERE_SPHERE = FIRST_SPHERE | SECOND_SPHERE,
	AABB_AABB = FIRST_AABB | SECOND_AABB,
	SPHERE_AABB = FIRST_SPHERE | SECOND_AABB,
	AABB_SPHERE = FIRST_AABB | SECOND_SPHERE,
	OBB_OBB = FIRST_OBB | SECOND_OBB,
	SPHERE_OBB = FIRST_SPHERE | SECOND_OBB,
	OBB_SPHERE = FIRST_OBB | SECOND_SPHERE,
	AABB_OBB = FIRST_AABB | SECOND_OBB,
	OBB_AABB = FIRST_OBB | SECOND_AABB,
	CAPSULE_CAPSULE = FIRST_CAPSULE | SECOND_CAPSULE,
	SPHERE_CAPSULE = FIRST_SPHERE | SECOND_CAPSULE,
	CAPSULE_SPHERE = FIRST_CAPSULE | SECOND_SPHERE,
	AABB_CAPSULE = FIRST_AABB | SECOND_CAPSULE,
	CAPSULE_AABB = FIRST_CAPSULE | SECOND_AABB,
	OBB_CAPSULE = FIRST_OBB | SECOND_CAPSULE,
	CAPSULE_OBB = FIRST_CAPSULE | SECOND_OBB
};

static CollisionFlag operator|(const CollisionFlag first, const CollisionFlag second) {
	return CollisionFlag(uint16_t(first) | uint16_t(second));
}

class PhysicsManager {
public:
	static PhysicsManager &get();

	const glm::vec3 &get_gravity();
	const float &get_epsilon();

	void resolve_collision(World &world, Entity movable_object, const std::set<Entity> &static_entities);

	glm::vec3 is_overlap(const ColliderSphere &a, const ColliderSphere &b);
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

	glm::vec3 is_overlap(const ColliderCapsule &a, const ColliderCapsule &b);
	void resolve_collision_capsule(World &world, Entity e1, Entity e2);

	glm::vec3 is_overlap(const ColliderCapsule &a, const ColliderSphere &b);
	void resolve_capsule_sphere(World &world, Entity capsule, Entity sphere);

	float closest_point_segment_segment(
			glm::vec3 p1, glm::vec3 q1, glm::vec3 p2, glm::vec3 q2, float &s, float &t, glm::vec3 &c1, glm::vec3 &c2);

	bool is_collision_candidate(const glm::vec3 &p1, const glm::vec3 &r1, const glm::vec3 &p2, const glm::vec3 &r2);
	void make_shift(World &world, Entity e1, Entity e2, const glm::vec3 &offset);
	void non_physical_shift(Transform &t1, Transform &t2, bool is_movable1, bool is_movable2, const glm::vec3 &offset);
	void physical_shift(Transform &t1, Transform &t2, RigidBody &b1, RigidBody &b2, bool is_movable1, bool is_movable2,
			const glm::vec3 &offset);

	// returns true, point and normal if ray intersect with sphere
	bool intersect_ray_sphere(const Ray &ray, const ColliderSphere &sphere, HitInfo &result);
	// returns true, point and normal if ray intersect with aabb
	bool intersect_ray_aabb(const Ray &ray, const ColliderAABB &aabb, HitInfo &result);
	// returns true, point and normal if ray intersect with obb
	bool intersect_ray_obb(const Ray &ray, const ColliderOBB &obb, HitInfo &result);

	void add_collision_layer(const std::string &layer_name);
	void remove_collision_layer(const std::string &layer_name);
	void set_layers_no_collision(const std::string &layer1, const std::string &layer2);
	void set_layers_collision(const std::string &layer1, const std::string &layer2);
	bool are_layers_collide(const std::string &layer1, const std::string &layer2);
	const std::unordered_map<std::string, std::unordered_set<std::string>> &get_layers_map();

private:
	// layers map that contains data about which layer does not collide with which ones
	// this is something like no collision map
	std::unordered_map<std::string, std::unordered_set<std::string>> layers_map;
	glm::vec3 gravity;
	float epsilon;
};

#endif //SILENCE_PHYSICS_MANAGER_H
