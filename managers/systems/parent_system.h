#ifndef SILENCE_PARENT_SYSTEM_H
#define SILENCE_PARENT_SYSTEM_H

#include "base_system.h"
#include "systems/isolated_entities_system.h"
#include "systems/root_parent_system.h"

// ParentSystem updates transform's model matrices of all entities using 2 other systems
class ParentSystem : public BaseSystem {
private:
	std::shared_ptr<IsolatedEntitiesSystem> isolated_entities_system;
	std::shared_ptr<RootParentSystem> root_parent_system;

public:
	void startup();
	void update();
};

#endif //SILENCE_PARENT_SYSTEM_H
