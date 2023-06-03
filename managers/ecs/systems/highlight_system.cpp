#include "ecs/systems/highlight_system.h"
#include "ecs/world.h"

void HighlightSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<Highlight>());

	world.set_system_component_whitelist<HighlightSystem>(whitelist);
}

void HighlightSystem::update(World &world, float dt) {
	ZoneScopedN("HighlightSystem::update");
	for (auto const &entity : entities) {
		auto &highlight_component = world.get_component<Highlight>(entity);

		if (highlight_component.highlighted) {
			highlight_component.highlighted = false;
		}

		
	}
}