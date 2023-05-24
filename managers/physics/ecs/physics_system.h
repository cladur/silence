#ifndef SILENCE_PHYSICS_SYSTEM_H
#define SILENCE_PHYSICS_SYSTEM_H

#include "managers/ecs/systems/base_system.h"

class PhysicsSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_PHYSICS_SYSTEM_H
