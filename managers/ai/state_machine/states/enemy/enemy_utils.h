#ifndef SILENCE_ENEMY_UTILS_H
#define SILENCE_ENEMY_UTILS_H

#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "managers/ecs/world.h"
#include "managers/gameplay/gameplay_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "managers/physics/physics_manager.h"
#include <audio/audio_manager.h>
#include <render/debug/debug_draw.h>
#include <render/transparent_elements/ui_manager.h>
#include <glm/vec3.hpp>

namespace enemy_utils {

	static glm::vec3 enemy_look_offset = glm::vec3(0.0f, 1.0f, 0.0f);
	static glm::vec3 agent_target_top_offset = glm::vec3(0.0f, 1.2f, 0.0f);
	static glm::vec3 agent_target_bottom_offset = glm::vec3(0.0f, 0.2f, 0.0f);

	inline void look_at(EnemyPath &path, Transform &t, glm::vec3 &target, float &dt) {
		if (path.first_rotation_frame) {
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

	inline void handle_detection(World *world, Transform &transform, glm::vec3 forward, EnemyData &enemy_data, float &dt, DebugDraw *dd = nullptr) {
		auto agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene()) + agent_target_top_offset;
		bool can_see_player = false;
		auto enemy_look_origin = transform.position + enemy_look_offset;
		auto agent_dir = glm::normalize(agent_pos - enemy_look_origin);
		//auto forward = glm::normalize(transform.get_global_forward());

		if (dd != nullptr) {
			//dd->draw_line(transform.position, transform.position + agent_dir, glm::vec3(1.0f, 0.0f, 0.0f));
			//dd->draw_line(transform.position, transform.position + forward, glm::vec3(0.0f, 1.0f, 0.0f));
//			dd->draw_cone(
//					transform.position + glm::vec3(0.0f, 1.0f, 0.0f),
//					transform.position + glm::vec3(0.0f, 1.0f, 0.0f) + forward,
//					enemy_data.view_cone_distance,
//					glm::tan(glm::radians(enemy_data.view_cone_angle) / 2.0f) * enemy_data.view_cone_distance,
//					glm::vec3(1.0f, 0.0f, 0.0f), 8);
		}

		if (glm::distance(transform.position, agent_pos) < enemy_data.view_cone_distance) {
			auto angle = glm::acos(glm::dot(agent_dir, forward));
			if (angle < glm::radians(enemy_data.view_cone_angle) / 2.0f) {
				Ray ray{};
				ray.origin = enemy_look_origin + agent_dir;
				ray.direction = agent_dir;
//				ray.ignore_layers.emplace_back("camera");
				glm::vec3 ray_end = ray.origin + ray.direction * enemy_data.view_cone_distance;

				HitInfo hit_info;

				// todo: make sure this checks properly for hitting a player and stops at terrain in between

				// RAY MIDDLE

				if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
					if (dd != nullptr) {
						dd->draw_line(ray.origin, ray_end, glm::vec3(0.0f, 1.0f, 1.0f));
					}
					auto &tag = world->get_component<ColliderTag>(hit_info.entity);
					if (hit_info.entity == GameplayManager::get().get_agent_entity()) {
						can_see_player = true;
					}
				}

				// RAY BOTTOM

				agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene()) + agent_target_bottom_offset;
				agent_dir = glm::normalize(agent_pos - enemy_look_origin);
				ray.direction = agent_dir;
				ray_end = ray.origin + ray.direction * enemy_data.view_cone_distance;

				if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
					if (dd != nullptr) {
						dd->draw_line(ray.origin, ray_end, glm::vec3(0.0f, 1.0f, 1.0f));
					}
					if (hit_info.entity == GameplayManager::get().get_agent_entity()) {
						can_see_player = true;
					}
				}

			}
		}
		if (can_see_player) {
			// if this is the first frame that the enemy sees the player, play a warning sound
			if (enemy_data.detection_level < dt / enemy_data.detection_speed) {
				AudioManager::get().play_one_shot_2d(EventReference("SFX/Enemies/first_time_detect"));
			}
			enemy_data.detection_level += dt / enemy_data.detection_speed;
		} else {
			enemy_data.detection_level -= dt / enemy_data.detection_speed;
		}

		enemy_data.detection_level = glm::clamp(enemy_data.detection_level, 0.0f, 1.0f);
		GameplayManager::get().add_detection_level(enemy_data.detection_level);
	}

	inline void update_detection_slider(uint32_t entity_id, Transform &transform, EnemyData &enemy_data) {
		auto &slider = UIManager::get().get_ui_slider(std::to_string(entity_id) + "_detection", "detection_slider");
		if (enemy_data.detection_level < 0.001f) {
			slider.display = false;
		} else {
			slider.display = true;
		}
		slider.value = enemy_data.detection_level;
		// lerp from white to red
		slider.color = glm::lerp(glm::vec4(1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), slider.value);
		slider.position = transform.get_global_position() + glm::vec3(0.0f, 2.5f, 0.0f);
	}

	inline uint32_t find_closest_node(World *world, glm::vec3 &position, Children &path) {
		uint32_t closest_node = 0;
		float closest_distance = 1000000.0f;

		for (int i = 0; i < path.children_count; i ++) {
			auto &node = world->get_component<Transform>(path.children[i]).position;
			auto distance = glm::distance(position, node);
			if (distance < closest_distance) {
				closest_distance = distance;
				closest_node = i;
			}
		}

		return closest_node;
	}

	inline void handle_highlight(uint32_t entity, World *world) {
		if (world->has_component<Highlight>(entity) && world->has_component<Taggable>(entity)) {
			auto &h = world->get_component<Highlight>(entity);
			auto &tag = world->get_component<Taggable>(entity);
			if (tag.tagged) {
				h.highlighted = true;
			}
		}
	}
}

#endif //SILENCE_ENEMY_UTILS_H
