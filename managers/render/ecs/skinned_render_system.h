#ifndef SILENCE_SKINNED_RENDER_SYSTEM_H
#define SILENCE_SKINNED_RENDER_SYSTEM_H
#include "ecs/systems/base_system.h"

class SkinnedRenderSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_SKINNED_RENDER_SYSTEM_H
