#ifndef SILENCE_ENEMY_UTILS_H
#define SILENCE_ENEMY_UTILS_H

#include "components/detection_camera_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "managers/ecs/world.h"
#include "managers/gameplay/gameplay_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "managers/physics/physics_manager.h"
#include <audio/audio_manager.h>
#include <render/debug/debug_draw.h>
#include <render/transparent_elements/ui_manager.h>
#include <cstdlib>
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

inline void handle_detection(World *world, uint32_t enemy_entity, Transform &transform, glm::vec3 forward,
		EnemyData &enemy_data, float &dt, DebugDraw *dd = nullptr) {
	// cvar stuff
	auto *cvar = CVarSystem::get();
	float cone_range = *cvar->get_float_cvar("enemy.detection_range");
	float cone_angle = *cvar->get_float_cvar("enemy.detection_angle");
	float min_speed = *cvar->get_float_cvar("enemy.min_detection_speed");
	float max_speed = *cvar->get_float_cvar("enemy.max_detection_speed");
	float sphere_radus = *cvar->get_float_cvar("enemy.sphere_detection_radius");
	float decrease_rate = *cvar->get_float_cvar("enemy.detection_decrease_speed");
	float crouch_mod = *cvar->get_float_cvar("enemy.crouch_detection_modifier");
	float hacker_mod = *cvar->get_float_cvar("enemy.hacker_detection_modifier");

	auto enemy_look_origin = transform.position + enemy_look_offset;

	auto agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene()) + agent_target_top_offset;
	bool can_see_player = false;
	auto agent_dir = glm::normalize(agent_pos - enemy_look_origin);
	float agent_distance_ratio = glm::distance(transform.position, agent_pos) / cone_range;

	auto hacker_pos = GameplayManager::get().get_hacker_position(world->get_parent_scene()) + agent_target_top_offset;
	auto hacker_dir = glm::normalize(hacker_pos - enemy_look_origin);
	bool can_see_hacker = false;
	float hacker_distance_ratio = glm::distance(transform.position, hacker_pos) / cone_range;

	// AGENT CONE DETECTION LOGIC
	if (glm::distance(transform.position, agent_pos) < cone_range) {
		auto angle = glm::acos(glm::dot(agent_dir, forward));
		if (angle < glm::radians(cone_angle) / 2.0f) {
			Ray ray{};
			ray.origin = enemy_look_origin + agent_dir;
			ray.direction = agent_dir;
			glm::vec3 ray_end = ray.origin + ray.direction * cone_range;

			HitInfo hit_info;

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

			agent_pos =
					GameplayManager::get().get_agent_position(world->get_parent_scene()) + agent_target_bottom_offset;
			agent_dir = glm::normalize(agent_pos - enemy_look_origin);
			ray.direction = agent_dir;
			ray_end = ray.origin + ray.direction * cone_range;

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

	// HACKER CONE DETECTION LOGIC
	if (glm::distance(transform.position, hacker_pos) < cone_range) {
		auto angle = glm::acos(glm::dot(hacker_dir, forward));
		if (angle < glm::radians(cone_angle) / 2.0f) {
			Ray ray{};
			ray.origin = enemy_look_origin + hacker_dir;
			ray.direction = hacker_dir;
			glm::vec3 ray_end = ray.origin + ray.direction * cone_range;

			HitInfo hit_info;

			// RAY MIDDLE

			if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
				if (dd != nullptr) {
					dd->draw_line(ray.origin, ray_end, glm::vec3(0.0f, 1.0f, 1.0f));
				}
				auto &tag = world->get_component<ColliderTag>(hit_info.entity);
				if (hit_info.entity == GameplayManager::get().get_hacker_entity()) {
					can_see_hacker = true;
				}
			}

			// RAY BOTTOM

			hacker_pos =
					GameplayManager::get().get_hacker_position(world->get_parent_scene()) + agent_target_bottom_offset;
			hacker_dir = glm::normalize(hacker_pos - enemy_look_origin);
			ray.direction = hacker_dir;
			ray.ignore_list.push_back(enemy_entity);
			ray_end = ray.origin + ray.direction * cone_range;

			if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
				if (hit_info.entity == GameplayManager::get().get_hacker_entity()) {
					can_see_hacker = true;
				}
			}
		}
	}

	// AGENT SPHERE DETECTION LOGIC
	if (glm::distance(transform.position, agent_pos) < sphere_radus) {
		Ray ray{};
		ray.origin = enemy_look_origin;
		ray.direction = agent_dir;
		ray.ignore_list.push_back(enemy_entity);
		glm::vec3 ray_end = ray.origin + ray.direction * sphere_radus;

		HitInfo hit_info;

		if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
			// only if agent is crouching then he can get to the enemy
			if (hit_info.entity == GameplayManager::get().get_agent_entity() &&
					!GameplayManager::get().get_agent_crouch()) {
				can_see_player = true;
			}
		}
	}

	if (can_see_player || can_see_hacker) {
		// if noone was detected past frame, play sound
		if (enemy_data.detection_target == DetectionTarget::NONE) {
			AudioManager::get().play_one_shot_2d(EventReference("SFX/Enemies/first_time_detect"));
		}

		// Agent has priority over hacker, because he is bigger and humanoid.
		enemy_data.detection_target = can_see_player ? DetectionTarget::AGENT : DetectionTarget::HACKER;
		float detection_speed = 0.0f;
		float detection_change = dt;

		// interpolate detection speed based on distance
		switch (enemy_data.detection_target) {
			case DetectionTarget::AGENT: {
				detection_speed = glm::mix(min_speed, max_speed, agent_distance_ratio);
				detection_change *= GameplayManager::get().get_agent_crouch() ? crouch_mod : 1.0f;
				break;
			}
			case DetectionTarget::HACKER: {
				detection_speed = glm::mix(min_speed, max_speed, hacker_distance_ratio);
				detection_change *= hacker_mod;
				break;
			}
			default: {
				break;
			}
		}

		detection_change *= detection_speed;

		enemy_data.detection_level += detection_change;
	} else {
		enemy_data.detection_target = DetectionTarget::NONE;
		enemy_data.detection_level -= dt * decrease_rate;
	}

	enemy_data.detection_level = glm::clamp(enemy_data.detection_level, 0.0f, 1.0f);
	GameplayManager::get().add_detection_level(enemy_data.detection_level);
}

inline void handle_detection_camera(World *world, uint32_t enemy_entity, Transform &transform, glm::vec3 forward,
		DetectionCamera &detection_camera, float &dt, DebugDraw *dd = nullptr) {
	// cvar stuff
	auto *cvar = CVarSystem::get();
	float cone_range = *cvar->get_float_cvar("enemy_camera.detection_range");
	float cone_angle = *cvar->get_float_cvar("enemy_camera.detection_angle");
	float vertical_cone_angle = *cvar->get_float_cvar("enemy_camera.detection_vertical_angle");
	float min_speed = *cvar->get_float_cvar("enemy.min_detection_speed");
	float max_speed = *cvar->get_float_cvar("enemy.max_detection_speed");
	float sphere_radius = *cvar->get_float_cvar("enemy.sphere_detection_radius");
	float decrease_rate = *cvar->get_float_cvar("enemy.detection_decrease_speed");
	float crouch_mod = *cvar->get_float_cvar("enemy.crouch_detection_modifier");
	float hacker_mod = *cvar->get_float_cvar("enemy.hacker_detection_modifier");

	glm::vec3 global_position = transform.get_global_position();

	auto enemy_look_origin = global_position;

	auto agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene()) + agent_target_top_offset;
	bool can_see_player = false;
	auto agent_dir = glm::normalize(agent_pos - enemy_look_origin);
	float agent_distance_ratio = glm::distance(global_position, agent_pos) / cone_range;

	auto hacker_pos = GameplayManager::get().get_hacker_position(world->get_parent_scene()) + agent_target_top_offset;
	auto hacker_dir = glm::normalize(hacker_pos - enemy_look_origin);
	bool can_see_hacker = false;
	float hacker_distance_ratio = glm::distance(global_position, hacker_pos) / cone_range;

	// AGENT CONE DETECTION LOGIC
	if (glm::distance(global_position, agent_pos) < cone_range) {
		auto angle = glm::acos(glm::dot(agent_dir, -forward));
		auto up_dot = glm::dot(agent_dir, glm::vec3(0.0f, 1.0f, 0.0f));

		bool in_horizontal_cone = angle < glm::radians(cone_angle) / 2.0f;
		bool in_vertical_cone = abs(up_dot) > cos(glm::radians(vertical_cone_angle) / 2.0f);
		if (in_vertical_cone && in_horizontal_cone) {
			Ray ray{};
			ray.origin = enemy_look_origin + agent_dir;
			ray.direction = agent_dir;
			glm::vec3 ray_end = ray.origin + ray.direction * cone_range;

			HitInfo hit_info;

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

			agent_pos =
					GameplayManager::get().get_agent_position(world->get_parent_scene()) + agent_target_bottom_offset;
			agent_dir = glm::normalize(agent_pos - enemy_look_origin);
			ray.direction = agent_dir;
			ray_end = ray.origin + ray.direction * cone_range;

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

	// HACKER CONE DETECTION LOGIC
	if (glm::distance(global_position, hacker_pos) < cone_range) {
		auto angle = glm::acos(glm::dot(hacker_dir, -forward));
		auto up_dot = glm::dot(hacker_dir, glm::vec3(0.0f, 1.0f, 0.0f));

		bool in_horizontal_cone = angle < glm::radians(cone_angle) / 2.0f;
		bool in_vertical_cone = abs(up_dot) > cos(glm::radians(vertical_cone_angle) / 2.0f);
		if (in_horizontal_cone && in_vertical_cone) {
			Ray ray{};
			ray.origin = enemy_look_origin + hacker_dir;
			ray.direction = hacker_dir;
			glm::vec3 ray_end = ray.origin + ray.direction * cone_range;

			HitInfo hit_info;

			// RAY MIDDLE

			if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
				if (dd != nullptr) {
					dd->draw_line(ray.origin, ray_end, glm::vec3(0.0f, 1.0f, 1.0f));
				}
				auto &tag = world->get_component<ColliderTag>(hit_info.entity);
				if (hit_info.entity == GameplayManager::get().get_hacker_entity()) {
					can_see_hacker = true;
				}
			}

			// RAY BOTTOM

			hacker_pos =
					GameplayManager::get().get_hacker_position(world->get_parent_scene()) + agent_target_bottom_offset;
			hacker_dir = glm::normalize(hacker_pos - enemy_look_origin);
			ray.direction = hacker_dir;
			ray.ignore_list.push_back(enemy_entity);
			ray_end = ray.origin + ray.direction * cone_range;

			if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
				if (hit_info.entity == GameplayManager::get().get_hacker_entity()) {
					can_see_hacker = true;
				}
			}
		}
	}

	// AGENT SPHERE DETECTION LOGIC
	if (glm::distance(global_position, agent_pos) < sphere_radius) {
		Ray ray{};
		ray.origin = enemy_look_origin;
		ray.direction = agent_dir;
		ray.ignore_list.push_back(enemy_entity);
		glm::vec3 ray_end = ray.origin + ray.direction * sphere_radius;

		HitInfo hit_info;

		if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
			// only if agent is crouching then he can get to the enemy
			if (hit_info.entity == GameplayManager::get().get_agent_entity() &&
					!GameplayManager::get().get_agent_crouch()) {
				can_see_player = true;
			}
		}
	}

	if (can_see_player || can_see_hacker) {
		// if noone was detected past frame, play sound
		if (detection_camera.detection_target == DetectionTarget::NONE) {
			AudioManager::get().play_one_shot_2d(EventReference("SFX/Enemies/first_time_detect"));
		}

		// Agent has priority over hacker, because he is bigger and humanoid.
		detection_camera.detection_target = can_see_player ? DetectionTarget::AGENT : DetectionTarget::HACKER;
		float detection_speed = 0.0f;
		float detection_change = dt;

		// interpolate detection speed based on distance
		switch (detection_camera.detection_target) {
			case DetectionTarget::AGENT: {
				detection_speed = glm::mix(min_speed, max_speed, agent_distance_ratio);
				detection_change *= GameplayManager::get().get_agent_crouch() ? crouch_mod : 1.0f;
				break;
			}
			case DetectionTarget::HACKER: {
				detection_speed = glm::mix(min_speed, max_speed, hacker_distance_ratio);
				detection_change *= hacker_mod;
				break;
			}
			default: {
				break;
			}
		}

		detection_change *= detection_speed;

		detection_camera.detection_level += detection_change;
	} else {
		detection_camera.detection_target = DetectionTarget::NONE;
		detection_camera.detection_level -= dt * decrease_rate;
	}

	detection_camera.detection_level = glm::clamp(detection_camera.detection_level, 0.0f, 1.0f);
	GameplayManager::get().add_detection_level(detection_camera.detection_level);
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

inline void update_detection_slider_camera(
		uint32_t entity_id, Transform &transform, DetectionCamera &detection_camera) {
	auto &slider = UIManager::get().get_ui_slider(std::to_string(entity_id) + "_detection", "detection_slider");
	if (detection_camera.detection_level < 0.001f) {
		slider.display = false;
	} else {
		slider.display = true;
	}
	slider.value = detection_camera.detection_level;
	// lerp from white to red
	slider.color = glm::lerp(glm::vec4(1.0f), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f), slider.value);
	slider.position = transform.get_global_position() + glm::vec3(0.0f, 2.5f, 0.0f);
}

inline uint32_t find_closest_node(World *world, glm::vec3 &position, Children &path) {
	uint32_t closest_node = 0;
	float closest_distance = 1000000.0f;

	for (int i = 0; i < path.children_count; i++) {
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

} //namespace enemy_utils

#endif //SILENCE_ENEMY_UTILS_H
