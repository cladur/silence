#include "ecs/systems/rotator_system.h"
#include "ecs/world.h"

void RotatorSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<Rotator>());
	whitelist.set(world.get_component_type<Transform>());

	world.set_system_component_whitelist<RotatorSystem>(whitelist);
}

void RotatorSystem::update(World &world, float dt) {
	ZoneScopedN("RotatorSystem::update");
	for (auto const &entity : entities) {
	}
}