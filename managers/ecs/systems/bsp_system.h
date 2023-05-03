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
	void startup();
	void update(class CollisionSystem &collision_system);
	void resolve_collision(BSPNode *node, Entity entity, class CollisionSystem &collision_system, bool force = false);

	std::shared_ptr<BSPNode> root;
	void build_tree(int32_t depth);
	void process_node(const std::set<Entity> &objects, BSPNode *node, int32_t depth);

	Plane calculate_plane(const std::set<Entity> &colliders);
	Side process_collider(const Plane &plane, const class ColliderOBB &collider);
	Side process_collider(const Plane &plane, const class ColliderSphere &collider);
	Side process_collider(const Plane &plane, const class ColliderAABB &collider);
};

#endif //SILENCE_BSP_SYSTEM_H