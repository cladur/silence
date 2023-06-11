#ifndef SILENCE_DECAL_SYSTEM_H
#define SILENCE_DECAL_SYSTEM_H

#include "ecs/systems/base_system.h"

class DecalSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_DECAL_SYSTEM_H
