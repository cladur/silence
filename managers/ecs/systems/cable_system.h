#ifndef SILENCE_CABLE_SYSTEM_H
#define SILENCE_CABLE_SYSTEM_H

#include "base_system.h"

class CableSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_CABLE_SYSTEM_H
