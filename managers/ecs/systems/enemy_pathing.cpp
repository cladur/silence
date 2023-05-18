#include "enemy_pathing.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include <components/enemy_path_component.h>
#include "glm/gtc/quaternion.hpp"

void look_at(EnemyPath &path, Transform &t, glm::vec3 &target, float &dt, DebugDraw &dd);

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
		enemy_path.prev_position = enemy_path.path[(enemy_path.next_position - 1) % enemy_path.path.size()];

		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		if (glm::distance(current_position, target_position) > 0.1f) {
			transform.add_position(glm::normalize(target_position - current_position) * enemy_path.speed * dt);
		} else {
			enemy_path.next_position = (enemy_path.next_position + 1) % enemy_path.path.size();
		}
		// this huge if just means "when near a node on either side"
		if (glm::distance(current_position, target_position) < (glm::distance(enemy_path.prev_position, target_position)) * 0.1f
				|| glm::distance(current_position, enemy_path.prev_position) < (glm::distance(enemy_path.prev_position, target_position)) * 0.1f) {
			SPDLOG_INFO("near node");
			if (!enemy_path.is_rotating) {
				enemy_path.first_rotation_frame = true;
			}
			enemy_path.is_rotating = true;
		}
		if (glm::dot(glm::normalize(target_position - transform.position), glm::normalize(transform.get_global_forward())) > 0.999f) {
			enemy_path.is_rotating = false;
		}
		if (enemy_path.is_rotating) {
			look_at(enemy_path, transform, target_position, dt, dd);
		}

		dd.draw_line(current_position, current_position + glm::normalize(transform.get_global_forward()), glm::vec3(0.0f, 0.0f, 1.0f));
		dd.draw_line(current_position, current_position + glm::normalize(target_position - transform.position), glm::vec3(1.0f, 0.0f, 0.0f));
	}
}

void look_at(EnemyPath &path, Transform &t, glm::vec3 &target, float &dt, DebugDraw &dd) {
	if (path.first_rotation_frame) {
		SPDLOG_INFO("calculating rotation");
		auto current_no_y = glm::vec3(t.position.x, 0.0f, t.position.z);
		auto target_no_y = glm::vec3(target.x, 0.0f, target.z);

		glm::vec3 direction = glm::normalize(target_no_y - current_no_y);
		glm::vec3 forward = glm::normalize(glm::vec3(t.get_global_forward().x, 0.0f, t.get_global_forward().z));
		float angle = glm::acos(glm::dot(forward, direction));
		glm::vec3 axis = glm::cross(forward, direction);
		path.rotation_end = (angle * axis);
		path.first_rotation_frame = false;
	}
	t.add_global_euler_rot(path.rotation_end * dt * path.rotation_speed);

}
