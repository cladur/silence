#ifndef SILENCE_PLATFORM_SYSTEM_H
#define SILENCE_PLATFORM_SYSTEM_H

#include "base_system.h"

class PlatformSystem : public BaseSystem {
private:
	float lerp(float a, float b, float f) {
		return a + f * (b - a);
	}
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_PLATFORM_SYSTEM_H