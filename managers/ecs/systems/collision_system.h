#ifndef SILENCE_COLLISION_SYSTEM_H
#define SILENCE_COLLISION_SYSTEM_H

#include "base_system.h"

struct ColliderAABB;

class CollisionSystem : public BaseSystem {
public:
	void startup();
	void update();

private:
	bool is_overlap(ColliderAABB &a, ColliderAABB &b);
	void resolve_collision_aabb(Entity e1, Entity e2);
};

#endif //SILENCE_COLLISION_SYSTEM_H
