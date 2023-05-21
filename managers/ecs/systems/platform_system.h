#ifndef SILENCE_PLATFORM_SYSTEM_H
#define SILENCE_PLATFORM_SYSTEM_H

#include "base_system.h"

class PlatformSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_PLATFORM_SYSTEM_H