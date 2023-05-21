#include "platform_system.h"
#include "ecs/world.h"

void PlatformSystem::startup(World &world) {
	Signature whitelist;
	whitelist.set(world.get_component_type<Platform>());

	world.set_system_component_whitelist<PlatformSystem>(whitelist);
}
void PlatformSystem::update(World &world, float dt) {
	for (auto const &entity : entities) {
		auto &platform = world.get_component<Platform>(entity);

		if (!platform.is_moving) {
			continue;
		}

		
	}
}