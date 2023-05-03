#ifndef SILENCE_RENDER_SYSTEM_H
#define SILENCE_RENDER_SYSTEM_H

#include "ecs/systems/base_system.h"

class RenderManager;

class RenderSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_RENDER_SYSTEM_H
