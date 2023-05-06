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
	void update(World &world);
	void resolve_collision(
			World &world, BSPNode *node, Entity entity, bool force = false);

	std::shared_ptr<BSPNode> root;
	void build_tree(World &world, int32_t depth);
	void process_node(World &world, const std::set<Entity> &objects, BSPNode *node, int32_t depth);

	Plane calculate_plane(World &world, const std::set<Entity> &colliders);
	Side process_collider(World &world, const Plane &plane, const class ColliderOBB &collider);
	Side process_collider(World &world, const Plane &plane, const class ColliderSphere &collider);
	Side process_collider(World &world, const Plane &plane, const class ColliderAABB &collider);
};

#endif //SILENCE_BSP_SYSTEM_H
