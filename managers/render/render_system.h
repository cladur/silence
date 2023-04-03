#ifndef SILENCE_RENDER_SYSTEM_H
#define SILENCE_RENDER_SYSTEM_H

#include "ecs/systems/base_system.h"

class RenderManager;

class RenderSystem : public BaseSystem {
public:
	void startup();
	void update(RenderManager &render_manager);
};

#endif //SILENCE_RENDER_SYSTEM_H
