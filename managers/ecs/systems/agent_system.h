#ifndef SILENCE_AGENT_SYSTEM_H
#define SILENCE_AGENT_SYSTEM_H

#include "base_system.h"

class AgentSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_AGENT_SYSTEM_H
