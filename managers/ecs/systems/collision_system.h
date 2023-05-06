#ifndef SILENCE_COLLISION_SYSTEM_H
#define SILENCE_COLLISION_SYSTEM_H

#include "base_system.h"

class CollisionSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_COLLISION_SYSTEM_H
