#ifndef SILENCE_PHYSICS_SYSTEM_H
#define SILENCE_PHYSICS_SYSTEM_H

#include "systems/base_system.h"
#include <list>

struct ColliderAABB;

class PhysicsSystem : public BaseSystem {
public:
	void startup();
	void update(float dt);
	void update_collision();

protected:
	friend class ColliderComponentsFactory;
	std::list<Entity> entities_with_collider;

private:
	bool is_overlap(ColliderAABB &a, ColliderAABB &b);
	void resolve_collision_aabb(Entity e1, Entity e2);
};

#endif //SILENCE_PHYSICS_SYSTEM_H
