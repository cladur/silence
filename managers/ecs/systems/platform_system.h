#ifndef SILENCE_PLATFORM_SYSTEM_H
#define SILENCE_PLATFORM_SYSTEM_H

#include "base_system.h"
#include <glm/exponential.hpp>

class PlatformSystem : public BaseSystem {
private:
	float lerp(float a, float b, float f) {
		f /= 4;
		f = sqrt(f);
		return a + f * (b - a);
	}

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_PLATFORM_SYSTEM_H