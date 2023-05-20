#ifndef SILENCE_INTERACTABLE_SYSTEM_H
#define SILENCE_INTERACTABLE_SYSTEM_H

#include "base_system.h"
#include "components/interactable_component.h"

class InteractableSystem : public BaseSystem {
private:
	static void no_interaction(World &world, Interactable &interactable, Entity entity);

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_INTERACTABLE_SYSTEM_H