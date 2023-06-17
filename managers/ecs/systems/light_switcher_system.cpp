#include "ecs/systems/light_switcher_system.h"
#include "components/light_switcher_component.h"
#include "ecs/world.h"


void LightSwitcherSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<LightSwitcher>());
	whitelist.set(world.get_component_type<Light>());

	world.set_system_component_whitelist<LightSwitcherSystem>(whitelist);
}

void LightSwitcherSystem::update(World &world, float dt) {
	ZoneScopedN("LightSwitcherSystem::update");
	for (auto const &entity : entities) {
	}
}