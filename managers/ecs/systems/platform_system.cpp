#include "platform_system.h"
#include "ecs/world.h"
#include <glm/fwd.hpp>

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
		glm::vec3 previous_position = platform_transform.get_position();
		glm::vec3 next_position = {0.0f, 0.0f, 0.0f};

		auto distance = glm::distance(platform_transform.get_global_position(), platform.ending_position);

		if (distance < 0.01f) {
			platform_transform.set_position(platform.ending_position);

			platform.is_moving = false;

			glm::vec3 temp = platform.starting_position;
			platform.starting_position = platform.ending_position;
			platform.ending_position = temp;
			next_position = platform.starting_position;
			
		} else {
			float speed = platform.speed * dt;
			next_position = {lerp(platform_transform.get_global_position().x, platform.ending_position.x, speed),
							lerp(platform_transform.get_global_position().y, platform.ending_position.y, speed),
							lerp(platform_transform.get_global_position().z, platform.ending_position.z, speed)};
			platform_transform.set_position(next_position);
		}
		platform.change_vector = next_position - previous_position;

	}
}