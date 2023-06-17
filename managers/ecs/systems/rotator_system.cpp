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
		auto &rotator = world.get_component<Rotator>(entity);
		auto &transform = world.get_component<Transform>(entity);

		if (rotator.is_rotating) {
			transform.add_euler_rot(glm::vec3(rotator.rotation_x, 0.0f, 0.0f) * dt);
			transform.add_global_euler_rot(glm::vec3(0.0f, rotator.rotation_y, 0.0f) * dt);
		}
	}
}