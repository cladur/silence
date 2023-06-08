#ifndef SILENCE_COLLISION_SYSTEM_H
#define SILENCE_COLLISION_SYSTEM_H

#include "managers/ecs/systems/base_system.h"
struct BSPNode;
struct Plane;
enum class Side;
struct Ray;
struct HitInfo;

class CollisionSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;

	void resolve_collision_dynamic(World &world);
	void resolve_bsp_collision(World &world, BSPNode *node, Entity entity, bool force = false);

	static std::shared_ptr<BSPNode> build_tree(World &world, std::vector<Entity> &world_entities, int32_t depth);
	static void process_node(World &world, const std::set<Entity> &objects, BSPNode *node, int32_t depth);

	static Plane calculate_plane(World &world, const std::set<Entity> &colliders);
	static Side process_collider(const Plane &plane, const class ColliderAABB &collider);
	static Side process_collider(const Plane &plane, const class ColliderOBB &collider);
	static Side process_collider(const Plane &plane, const class ColliderSphere &collider);
	static Side process_collider(const Plane &plane, const class ColliderCapsule &collider);
	static void log_tree(BSPNode *node);

	// Cast ray that collide with first intersected collider excluding layers conditions
	static bool ray_cast(World &world, const Ray &ray, HitInfo &result);
	// Cast ray that collide with first intersected collider including ray layer conditions
	static bool ray_cast_layer(World &world, const Ray &ray, HitInfo &result);
};

#endif //SILENCE_COLLISION_SYSTEM_H
