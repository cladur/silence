#include "enemy_pathing.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include <components/enemy_path_component.h>

void EnemyPathing::startup(World &world) {
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());

	world.set_system_component_whitelist<EnemyPathing>(whitelist);
}

void EnemyPathing::update(World &world, float dt) {
	auto r_s = world.get_parent_scene()->get_render_scene();
	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &enemy_path = world.get_component<EnemyPath>(entity);

		glm::vec3 current_position = transform.position;
		glm::vec3 target_position = enemy_path.path[enemy_path.next_position];

	}
}
