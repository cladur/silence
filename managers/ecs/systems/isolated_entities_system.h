#ifndef SILENCE_ISOLATED_ENTITIES_SYSTEM_H
#define SILENCE_ISOLATED_ENTITIES_SYSTEM_H

#include "base_system.h"

// IsolatedEntitiesSystem updates matrices of all entities that have no children and parent.
class IsolatedEntitiesSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_ISOLATED_ENTITIES_SYSTEM_H
