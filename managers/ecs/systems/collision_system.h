#ifndef SILENCE_COLLISION_SYSTEM_H
#define SILENCE_COLLISION_SYSTEM_H

#include "base_system.h"

struct ColliderAABB;
struct ColliderSphere;

class CollisionSystem : public BaseSystem {
public:
	void startup();
	void update();

private:
	bool is_overlap(ColliderAABB &a, ColliderAABB &b);
	void resolve_collision_aabb(Entity e1, Entity e2);

	bool is_overlap(ColliderSphere &a, ColliderSphere &b);
	void resolve_collision_sphere(Entity e1, Entity e2);

	bool is_overlap(ColliderAABB &aabb, ColliderSphere &sphere);
	void resolve_aabb_sphere(Entity aabb, Entity sphere);
};

#endif //SILENCE_COLLISION_SYSTEM_H
