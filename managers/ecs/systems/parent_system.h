#ifndef SILENCE_PARENT_SYSTEM_H
#define SILENCE_PARENT_SYSTEM_H

#include "base_system.h"
#include "isolated_entities_system.h"
#include "root_parent_system.h"

// ParentSystem updates transform's model matrices of all entities using 2 other systems
class ParentSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_PARENT_SYSTEM_H
