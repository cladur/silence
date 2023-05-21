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

		auto &platform_transform = world.get_component<Transform>(entity);

		if (glm::distance(platform_transform.get_global_position(), platform.ending_position) < 0.1f) {
			platform_transform.set_position(platform.ending_position);

			platform.is_moving = false;

			glm::vec3 temp = platform.starting_position;
			platform.starting_position = platform.ending_position;
			platform.ending_position = temp;
		} else {
			float speed = platform.speed * dt;
			platform_transform.set_position(
					glm::vec3(lerp(platform_transform.get_global_position().x, platform.ending_position.x, speed),
							lerp(platform_transform.get_global_position().y, platform.ending_position.y, speed),
							lerp(platform_transform.get_global_position().z, platform.ending_position.z, speed)));
		}
	}
}