//
// Created by Kurogami on 24/03/2023.
//

#ifndef SILENCE_PHYSICS_SYSTEM_H
#define SILENCE_PHYSICS_SYSTEM_H

#include "ecs/base_system.h"
class PhysicsSystem : public BaseSystem {
public:
	void startup();
	void update(float dt);
};

#endif //SILENCE_PHYSICS_SYSTEM_H
