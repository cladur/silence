#include "enemy_pathing.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include <components/enemy_path_component.h>

void EnemyPathing::startup(World &world) {
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<EnemyPath>());

	world.set_system_component_whitelist<EnemyPathing>(whitelist);
}

void EnemyPathing::update(World &world, float dt) {
	auto r_s = world.get_parent_scene()->get_render_scene();
	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &enemy_path = world.get_component<EnemyPath>(entity);

		glm::vec3 current_position = transform.position;
		glm::vec3 target_position = enemy_path.path[enemy_path.next_position];

		if (glm::distance(current_position, target_position) > 0.1f) {
			SPDLOG_INFO("Enemy Pathing: {} -> {}", glm::to_string(current_position), glm::to_string(target_position));
			transform.add_position(glm::normalize(target_position - current_position) * enemy_path.speed * dt);

		} else {
			enemy_path.next_position = (enemy_path.next_position + 1) % enemy_path.path.size();
		}

	}
}
