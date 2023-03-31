#ifndef SILENCE_PHYSICS_SYSTEM_H
#define SILENCE_PHYSICS_SYSTEM_H

#include "systems/base_system.h"

struct ColliderAABB;

class PhysicsSystem : public BaseSystem {
public:
	void startup();
	void update(float dt);
	void update_collision();

private:
	bool is_overlap(ColliderAABB &a, ColliderAABB &b);
	void resolve_collision(ColliderAABB &a, ColliderAABB &b);
};

#endif //SILENCE_PHYSICS_SYSTEM_H
