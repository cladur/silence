#include "enemy_pathing.h"
#include "components/enemy_data_component.h"
#include "ecs/systems/enemy_pathing.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include "glm/gtc/quaternion.hpp"
#include <components/enemy_path_component.h>
#include <spdlog/spdlog.h>

void EnemyPathing::startup(World &world) {
	Signature whitelist;
	Signature blacklist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<EnemyPath>());

	blacklist.set(world.get_component_type<EnemyData>());

	world.set_system_component_whitelist<EnemyPathing>(whitelist);
	world.set_system_component_blacklist<EnemyPathing>(blacklist);
}

void EnemyPathing::update(World &world, float dt) {
	ZoneScopedN("EnemyPathing::update");
	auto r_s = world.get_parent_scene()->get_render_scene();
	if (world.get_parent_scene()->frame_number != 0) {
		for (const Entity entity : entities) {
			auto &transform = world.get_component<Transform>(entity);
			auto &enemy_path = world.get_component<EnemyPath>(entity);
			auto &path = world.get_component<Children>(enemy_path.path_parent);

			if (enemy_path.patrol_cooldown > 0.0f) {
				enemy_path.patrol_cooldown -= dt;
				if (enemy_path.patrol_cooldown <= 0.0f) {
					enemy_path.next_position = (enemy_path.next_position + 1) % path.children_count;
				}
			} else {
				if (enemy_path.next_position == 0) {
					transform.set_position(world.get_component<Transform>(path.children[enemy_path.next_position])
												   .get_global_position());
					enemy_path.next_position = 1;
				} else {
					glm::vec3 current_position = transform.get_global_position();
					glm::vec3 target_position = world.get_component<Transform>(path.children[enemy_path.next_position])
														.get_global_position();

					// get index of previous node
					int idx = ((enemy_path.next_position - 1) % (int)path.children_count == -1)
							? path.children_count - 1
							: (enemy_path.next_position - 1) % path.children_count;

					auto &next_node = world.get_component<PathNode>(path.children[enemy_path.next_position]);
					auto &prev_node = world.get_component<PathNode>(path.children[idx]);

					enemy_path.prev_position = world.get_component<Transform>(path.children[idx]).get_global_position();

					// move the entity towards the next node
					if (glm::distance(current_position, target_position) > 0.1f) {
						transform.add_position(
								glm::normalize(target_position - current_position) * enemy_path.speed * dt);
						// if the point is a patrol point, switch state
					} else if (next_node.is_patrol_point) {
						enemy_path.patrol_cooldown = next_node.patrol_time;
						// if the node is not patrol node, just move to the next node
					} else {
						enemy_path.next_position = (enemy_path.next_position + 1) % path.children_count;
					}

					// this huge if just means "when near a node on either side" start rotating
					if (glm::distance(current_position, target_position) <
									(glm::distance(enemy_path.prev_position, target_position)) * 0.1f ||
							glm::distance(current_position, enemy_path.prev_position) <
									(glm::distance(enemy_path.prev_position, target_position)) * 0.1f) {
						if (!enemy_path.is_rotating) {
							enemy_path.first_rotation_frame = true;
						}
						enemy_path.is_rotating = true;
					}

					// smoothly rotate the entity to face the next node
					if (enemy_path.is_rotating) {
						glm::vec3 direction = transform.get_global_position() - target_position;
						direction.y = 0.0f;
						direction = glm::normalize(direction);

						glm::quat target_orientation = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f));

						transform.set_orientation(
								glm::slerp(transform.get_orientation(), target_orientation, 5.0f * dt));
					}

					// if the entity is facing the next node, stop rotating
					if (glm::dot(glm::normalize(target_position - transform.position),
								glm::normalize(transform.get_global_forward())) > 0.99f) {
						enemy_path.is_rotating = false;
					}
				}
			}
		}
	}
}