#ifndef SILENCE_LIGHT_RENDER_SYSTEM_H
#define SILENCE_LIGHT_RENDER_SYSTEM_H

#include "ecs/systems/base_system.h"

class LightRenderSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_LIGHT_RENDER_SYSTEM_H
