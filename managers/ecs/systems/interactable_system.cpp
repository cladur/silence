#include "interactable_system.h"
#include "ecs/world.h"

void InteractableSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Interactable>());
	world.set_system_component_whitelist<InteractableSystem>(signature);
}

void InteractableSystem::update(World &world, float dt) {
	for (auto const &entity : entities) {
		auto &interactable = world.get_component<Interactable>(entity);

		if (interactable.triggered) {
			interactable.triggered = false;

			switch (interactable.interaction) {
				case Interaction::NoInteraction:
					no_interaction(world, interactable, entity);
					break;
				case HackerCameraJump:
					interactable.triggered = false;
					break;
			}
		}
	}
}

void InteractableSystem::no_interaction(World &world, Interactable &interactable, Entity entity) {
	SPDLOG_WARN("No interaction set for entity {}, turning off interactions on this object", entity);
	interactable.can_interact = false;
}
