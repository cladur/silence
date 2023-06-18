#include "enemy_pathing.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include "glm/gtc/quaternion.hpp"
#include <components/enemy_path_component.h>

void EnemyPathing::startup(World &world) {
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<EnemyPath>());

	world.set_system_component_whitelist<EnemyPathing>(whitelist);
}

void EnemyPathing::update(World &world, float dt) {
	ZoneScopedN("EnemyPathing::update");
	auto r_s = world.get_parent_scene()->get_render_scene();
	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &enemy_path = world.get_component<EnemyPath>(entity);

		//		glm::vec3 current_position = transform.position;
		//		glm::vec3 target_position = enemy_path.path[enemy_path.next_position];
		//		int idx = ((enemy_path.next_position - 1) % (int)enemy_path.path.size() == -1)
		//				? enemy_path.path.size() - 1
		//				: (enemy_path.next_position - 1) % enemy_path.path.size();
		//		enemy_path.prev_position = enemy_path.path[idx];
		//
		//		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;
		//
		//		if (glm::distance(current_position, target_position) > 0.1f) {
		//			transform.add_position(glm::normalize(target_position - current_position) * enemy_path.speed * dt);
		//		} else {
		//			enemy_path.next_position = (enemy_path.next_position + 1) % enemy_path.path.size();
		//		}

		// this huge if just means "when near a node on either side" start rotating
		//		if (glm::distance(current_position, target_position) <
		//						(glm::distance(enemy_path.prev_position, target_position)) * 0.1f ||
		//				glm::distance(current_position, enemy_path.prev_position) <
		//						(glm::distance(enemy_path.prev_position, target_position)) * 0.1f) {
		//			if (!enemy_path.is_rotating) {
		//				enemy_path.first_rotation_frame = true;
		//			}
		//			enemy_path.is_rotating = true;
		//		}
		//
		//		// smoothly rotate the entity to face the next node
		//		if (enemy_path.is_rotating) {
		//			look_at(enemy_path, transform, target_position, dt, dd);
		//		}
		//
		//		// if the entity is facing the next node, stop rotating
		//		if (glm::dot(glm::normalize(target_position - transform.position),
		//					glm::normalize(transform.get_global_forward())) > 0.99f) {
		//			enemy_path.is_rotating = false;
		//		}
		//
		//		dd.draw_line(current_position, current_position + glm::normalize(transform.get_global_forward()),
		//				glm::vec3(0.0f, 0.0f, 1.0f));
		//		dd.draw_line(current_position, current_position + glm::normalize(target_position - transform.position),
		//				glm::vec3(1.0f, 0.0f, 0.0f));
	}
}