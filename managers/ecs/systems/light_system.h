#ifndef SILENCE_LIGHT_SYSTEM_H
#define SILENCE_LIGHT_SYSTEM_H

#include "base_system.h"

class LightSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_LIGHT_SYSTEM_H
