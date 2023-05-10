#ifndef SILENCE_FRUSTUM_DRAW_SYSTEM_H
#define SILENCE_FRUSTUM_DRAW_SYSTEM_H

#include "ecs/systems/base_system.h"

class RenderManager;

class FrustumDrawSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_FRUSTUM_DRAW_SYSTEM_H
