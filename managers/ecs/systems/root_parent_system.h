#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-extra-qualification"
#ifndef SILENCE_ROOT_PARENT_SYSTEM_H
#define SILENCE_ROOT_PARENT_SYSTEM_H

#include "base_system.h"

// This system finds all entities that have only children and no parent, updates their matrices and then updates
// children matrices using previously calculated matrix going down to all children recursively
class RootParentSystem : public BaseSystem {
public:
	void startup();
	void update();

private:
	void update_children(Entity parent, glm::mat4 parent_model);
};

#endif //SILENCE_ROOT_PARENT_SYSTEM_H