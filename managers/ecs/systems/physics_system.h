#ifndef SILENCE_PHYSICS_SYSTEM_H
#define SILENCE_PHYSICS_SYSTEM_H

#include "base_system.h"
class PhysicsSystem : public BaseSystem {
public:
	void startup();
	void update(float dt);
};

#endif //SILENCE_PHYSICS_SYSTEM_H
