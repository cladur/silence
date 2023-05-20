#ifndef SILENCE_ATTACHMENT_SYSTEM_H
#define SILENCE_ATTACHMENT_SYSTEM_H
#include "ecs/systems/base_system.h"

// This system finds all entities that have only children and no parent, updates their matrices and then updates
// children matrices using previously calculated matrix going down to all children recursively
class AttachmentSystem : public BaseSystem {
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_ATTACHMENT_SYSTEM_H
