#ifndef SILENCE_INTERACTABLE_SYSTEM_H
#define SILENCE_INTERACTABLE_SYSTEM_H

#include "base_system.h"
#include "components/interactable_component.h"
#include <audio/event_reference.h>

class InteractableSystem : public BaseSystem {
private:
	EventReference explostion_event;
	EventReference electric_interaction_event;

	void no_interaction(World &world, Interactable &interactable, Entity entity);
	void explosion(World &world, Interactable &interactable, Entity entity);
	void switch_light(World &world, Entity light_entity);

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_INTERACTABLE_SYSTEM_H