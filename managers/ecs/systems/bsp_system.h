#ifndef SILENCE_BSP_SYSTEM_H
#define SILENCE_BSP_SYSTEM_H

#include "base_system.h"

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

class BSPSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
	void resolve_collision(
			World &world, BSPNode *node, Entity entity, bool force = false);

	//std::shared_ptr<BSPNode> root;
	static std::shared_ptr<BSPNode> build_tree(World &world, std::vector<Entity> world_entities, int32_t depth);
	static void process_node(World &world, const std::set<Entity> &objects, BSPNode *node, int32_t depth);

	static Plane calculate_plane(World &world, const std::set<Entity> &colliders);
	static Side process_collider(World &world, const Plane &plane, const class ColliderOBB &collider);
	static Side process_collider(World &world, const Plane &plane, const class ColliderSphere &collider);
	static Side process_collider(World &world, const Plane &plane, const class ColliderAABB &collider);
	static void log_tree(BSPNode *node);
};

#endif //SILENCE_BSP_SYSTEM_H
