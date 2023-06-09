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
	void switch_rotator(World &world, Entity &light_entity);
	void switch_light(World &world, Entity light_entity);
	void switch_light_temporal(World &world, const std::vector<Entity> &light_entities, Interactable &interactable,
			float dt, const std::vector<Entity> &enemy_entities = {});
	void rotate_handle(World &world, float &dt, Interactable &interactable);
	void switch_lights(World &world, Interactable &interactable);

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_INTERACTABLE_SYSTEM_H