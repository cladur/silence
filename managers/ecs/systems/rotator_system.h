#ifndef SILENCE_ROTATOR_SYSTEM_H
#define SILENCE_ROTATOR_SYSTEM_H

#include "base_system.h"

class RotatorSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_ROTATOR_SYSTEM_H
