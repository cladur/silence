#ifndef SILENCE_ENEMY_UTILS_H
#define SILENCE_ENEMY_UTILS_H

#include "components/detection_camera_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "managers/ecs/world.h"
#include "managers/gameplay/gameplay_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "managers/physics/physics_manager.h"
#include <animation/animation_manager.h>
#include <audio/audio_manager.h>
#include <render/debug/debug_draw.h>
#include <render/transparent_elements/ui_manager.h>
#include <cstdlib>
#include <engine/scene.h>
#include <glm/vec3.hpp>
#include <glm/gtx/rotate_vector.hpp>

namespace enemy_utils {
static const glm::vec3 enemy_look_offset = glm::vec3(0.0f, 1.0f, 0.0f);
static const glm::vec3 agent_target_top_offset = glm::vec3(0.0f, 1.2f, 0.0f);
static const glm::vec3 agent_target_bottom_offset = glm::vec3(0.0f, 0.2f, 0.0f);
static const float footstep_left_down_threshold = 0.007f;
static const float footstep_right_down_threshold = 0.009f;
static const float footstep_up_threshold = 0.1f;
static std::string left_foot_bone = "heel.02.L";
static std::string right_foot_bone = "heel.02.R";

inline glm::vec2 transform_to_screen(const glm::vec3& position, const RenderScene &scene, bool is_right) {
	// Model-View-Projection transformation
	glm::vec2 render_extent = scene.render_extent;

	glm::mat4 mvp;

	if (is_right) {
		mvp = scene.projection * scene.view;
	} else {
		mvp = scene.left_projection * scene.left_view;
	}

	// Transform position to clip space
	glm::vec4 clip_space = mvp * glm::vec4(position, 1.0f);

	// Perspective divide to bring position to normalized device coordinates (NDC)
	glm::vec3 ndcSpacePosition = glm::vec3(clip_space) / clip_space.w;

	// Convert NDC to screen space coordinates
	float x = (ndcSpacePosition.x) * (render_extent.x / 2.0f);
	float y = (ndcSpacePosition.y) * (render_extent.y / 2.0f);

	return glm::vec2(x, y);
}

inline void handle_detection(World *world, uint32_t enemy_entity, Transform &transform, glm::vec3 forward,
		EnemyData &enemy_data, float &dt, DebugDraw *dd = nullptr) {
	// cvar stuff
	auto *cvar = CVarSystem::get();
	float cone_range = *cvar->get_float_cvar("enemy.detection_range");
	float cone_angle = *cvar->get_float_cvar("enemy.detection_angle");
	float min_speed = *cvar->get_float_cvar("enemy.min_detection_speed");
	float max_speed = *cvar->get_float_cvar("enemy.max_detection_speed");
	float sphere_radius = *cvar->get_float_cvar("enemy.sphere_detection_radius");
	float decrease_rate = *cvar->get_float_cvar("enemy.detection_decrease_speed");
	float crouch_mod = *cvar->get_float_cvar("enemy.crouch_detection_modifier");
	float hacker_mod = *cvar->get_float_cvar("enemy.hacker_detection_modifier");

	bool is_blinded = enemy_data.is_blinded;

	glm::vec3 current_global_position = transform.get_global_position();

	auto enemy_look_origin = current_global_position + enemy_look_offset;

	auto agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene()) + agent_target_top_offset;
	bool can_see_player = false;
	auto agent_dir = glm::normalize(agent_pos - enemy_look_origin);
	float agent_distance_ratio = glm::distance(current_global_position, agent_pos) / cone_range;

	auto hacker_pos = GameplayManager::get().get_hacker_position(world->get_parent_scene());
	auto hacker_dir = glm::normalize(hacker_pos - enemy_look_origin);
	bool can_see_hacker = false;
	float hacker_distance_ratio = glm::distance(current_global_position, hacker_pos) / cone_range;

	// AGENT CONE DETECTION LOGIC
	if (!is_blinded && glm::distance(current_global_position, agent_pos) < cone_range) {
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

			agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene()) + agent_target_bottom_offset;
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
	if (!is_blinded && glm::distance(current_global_position, hacker_pos) < cone_range) {
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
	if (glm::distance(transform.position + transform.get_global_forward() * 1.5f, agent_pos) < sphere_radius) {
		Ray ray{};
		ray.origin = enemy_look_origin;
		ray.direction = glm::normalize(agent_pos - ray.origin);
		ray.ignore_list.push_back(enemy_entity);

		HitInfo hit_info;

		dd->draw_sphere(transform.position + transform.get_global_forward() * 1.5f, sphere_radius, glm::vec3(1.0f, 0.0f, 0.0f));

		if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
			if (hit_info.entity == GameplayManager::get().get_agent_entity()) {
				can_see_player = true;
			}
		}
	}

	// HACKER SPHERE DETECTION LOGIC
	if (glm::distance(transform.position + transform.get_global_forward() * 1.5f, hacker_pos) < sphere_radius) {
		Ray ray{};
		ray.origin = enemy_look_origin;
		ray.direction = glm::normalize(hacker_pos - ray.origin);
		ray.ignore_list.push_back(enemy_entity);

		HitInfo hit_info;

		dd->draw_sphere(transform.position + transform.get_global_forward() * 1.5f, sphere_radius, glm::vec3(1.0f, 0.0f, 0.0f));

		if (CollisionSystem::ray_cast_layer(*world, ray, hit_info)) {
			if (hit_info.entity == GameplayManager::get().get_hacker_entity()) {
				can_see_hacker = true;
			}
		}
	}

	if (can_see_player || can_see_hacker) {
		// if noone was detected past frame, play sound
		auto &agent_screen_flash = UIManager::get().get_ui_image(std::to_string(enemy_entity) + "_detection", "agent_detection_screen_flash");
		auto &hacker_screen_flash = UIManager::get().get_ui_image(std::to_string(enemy_entity) + "_detection", "hacker_detection_screen_flash");

		bool prev_none = false;
		if (enemy_data.detection_target == DetectionTarget::NONE) {
			prev_none = true;
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
				if (prev_none) {
					agent_screen_flash.display = true;
					agent_screen_flash.size = world->get_parent_scene()->get_render_scene().render_extent;
					agent_screen_flash.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
				}
				break;
			}
			case DetectionTarget::HACKER: {
				detection_speed = glm::mix(min_speed, max_speed, hacker_distance_ratio);
				detection_change *= hacker_mod;
				if (prev_none) {
					hacker_screen_flash.display = true;
					hacker_screen_flash.size = world->get_parent_scene()->get_render_scene().render_extent;
					hacker_screen_flash.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
				}
				break;
			}
			default: {
				break;
			}
		}

		auto new_agent_flash_color = agent_screen_flash.color;
		auto new_hacker_flash_color = hacker_screen_flash.color;
		new_agent_flash_color.a -= dt * 3.0f;
		new_hacker_flash_color.a -= dt * 3.0f;
		new_agent_flash_color.a = glm::max(new_agent_flash_color.a, 0.0f);
		new_hacker_flash_color.a = glm::max(new_hacker_flash_color.a, 0.0f);
		agent_screen_flash.color = new_agent_flash_color;
		hacker_screen_flash.color = new_hacker_flash_color;

		if (new_agent_flash_color.a <= 0.001f) {
			agent_screen_flash.display = false;
		}
		if (new_hacker_flash_color.a <= 0.001f) {
			hacker_screen_flash.display = false;
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
	if (glm::distance(global_position, hacker_pos) < cone_range) {
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

			hacker_pos = GameplayManager::get().get_hacker_position(world->get_parent_scene());
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

	if (can_see_player || can_see_hacker) {
		detection_camera.is_detecting = true;

		auto &agent_screen_flash = UIManager::get().get_ui_image(std::to_string(enemy_entity) + "_detection", "agent_detection_screen_flash");
		auto &hacker_screen_flash = UIManager::get().get_ui_image(std::to_string(enemy_entity) + "_detection", "hacker_detection_screen_flash");

		bool prev_none = false;
		if (detection_camera.detection_target == DetectionTarget::NONE) {
			AudioManager::get().play_one_shot_2d(EventReference("SFX/Enemies/first_time_detect"));
			prev_none = true;
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
				if (prev_none) {
					agent_screen_flash.display = true;
					agent_screen_flash.size = world->get_parent_scene()->get_render_scene().render_extent;
					agent_screen_flash.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
				}
				break;
			}
			case DetectionTarget::HACKER: {
				detection_speed = glm::mix(min_speed, max_speed, hacker_distance_ratio);
				detection_change *= hacker_mod;
				if (prev_none) {
					hacker_screen_flash.display = true;
					hacker_screen_flash.size = world->get_parent_scene()->get_render_scene().render_extent;
					hacker_screen_flash.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.5f);
				}
				break;
			}
			default: {
				break;
			}
		}

		auto new_agent_flash_color = agent_screen_flash.color;
		auto new_hacker_flash_color = hacker_screen_flash.color;
		new_agent_flash_color.a -= dt * 3.0f;
		new_hacker_flash_color.a -= dt * 3.0f;
		new_agent_flash_color.a = glm::max(new_agent_flash_color.a, 0.0f);
		new_hacker_flash_color.a = glm::max(new_hacker_flash_color.a, 0.0f);
		agent_screen_flash.color = new_agent_flash_color;
		hacker_screen_flash.color = new_hacker_flash_color;

		if (!detection_camera.is_playing) {
			AudioManager::get().play_local(detection_camera.detection_event);
			detection_camera.is_playing = true;
		}

		FMOD_3D_ATTRIBUTES attributes = AudioManager::to_3d_attributes(transform);
		detection_camera.detection_event->set3DAttributes(&attributes);

		detection_change *= detection_speed;

		detection_camera.detection_level += detection_change;
	} else {
		detection_camera.is_detecting = false;

		if (detection_camera.is_playing) {
			AudioManager::get().stop_local(detection_camera.detection_event);
			detection_camera.is_playing = false;
		}

		detection_camera.detection_target = DetectionTarget::NONE;
		detection_camera.detection_level -= dt * decrease_rate;
	}

	detection_camera.detection_level = glm::clamp(detection_camera.detection_level, 0.0f, 1.0f);
	GameplayManager::get().add_detection_level(detection_camera.detection_level);
}

inline void update_detection_slider(uint32_t entity_id, Transform &transform, EnemyData &enemy_data, RenderScene &r_scene, Scene *scene) {
	auto &agent_detection_outline = UIManager::get().get_ui_image(std::to_string(entity_id) + "_detection", "agent_detection_outline");
	auto &hacker_detection_outline = UIManager::get().get_ui_image(std::to_string(entity_id) + "_detection", "hacker_detection_outline");
	auto &agent_detection_fill = UIManager::get().get_ui_image(std::to_string(entity_id) + "_detection", "agent_detection_fill");
	auto &hacker_detection_fill = UIManager::get().get_ui_image(std::to_string(entity_id) + "_detection", "hacker_detection_fill");

	auto &window_size = r_scene.render_extent;
	auto agent_pos = GameplayManager::get().get_agent_position(scene);
	auto hacker_pos = GameplayManager::get().get_hacker_position(scene);

	auto distance_to_agent = glm::clamp(glm::distance(transform.get_global_position(), agent_pos), 1.0f, 100.0f);
	auto distance_to_hacker = glm::clamp(glm::distance(transform.get_global_position(), hacker_pos), 1.0f, 100.0f);

	agent_detection_outline.is_screen_space = true;
	agent_detection_outline.is_billboard = false;

	agent_detection_fill.is_screen_space = true;
	agent_detection_fill.is_billboard = false;

	auto agent_detection_pos = transform_to_screen(transform.get_global_position() + glm::vec3(0.0f, 2.0f, 0.0f), r_scene, false);
	agent_detection_outline.position = glm::vec3(agent_detection_pos,0.1f);
	agent_detection_fill.position = glm::vec3(agent_detection_pos + glm::vec2(0.0f, 1.0f),0.0f);

	agent_detection_outline.size = enemy_data.detection_slider_default_size / 2.5f + enemy_data.detection_slider_default_size / (distance_to_agent);
	agent_detection_fill.size = glm::lerp(glm::vec2(0.0f), agent_detection_outline.size * 0.95f, enemy_data.detection_level);
	if (enemy_data.detection_level < 0.4f) {
		agent_detection_fill.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		agent_detection_outline.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	} else if (enemy_data.detection_level >= 0.4f) {
		agent_detection_fill.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		agent_detection_outline.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}

	if (enemy_data.detection_level >= 0.99f) {
		agent_detection_fill.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		agent_detection_outline.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	auto agent_pos_no_y = agent_pos;
	agent_pos_no_y.y = 0.0f;
	auto target_pos_no_y = transform.get_global_position();
	target_pos_no_y.y = 0.0f;
	auto dir = glm::normalize(agent_pos_no_y - target_pos_no_y);
	auto cam_forward = scene->world.get_component<Transform>(GameplayManager::get().get_agent_camera(scene)).get_global_forward();
	auto cam_forward_xz_proj = glm::normalize(glm::vec3(cam_forward.x, 0.0f, cam_forward.z));

	auto angle = glm::degrees(glm::acos(glm::dot(dir, cam_forward_xz_proj)));

	if (
			agent_detection_outline.position.x > window_size.x / 2.0f ||
			agent_detection_outline.position.y > window_size.y / 2.0f ||
			agent_detection_outline.position.x < -window_size.x / 2.0f ||
			agent_detection_outline.position.y < -window_size.y / 2.0f ||
			angle > 90.0f
			) {
		// check if rotation should be more than 180 degrees
		auto cross = glm::cross(dir, cam_forward_xz_proj);
		if (cross.y > 0.0f) {
			angle = -angle;
		}
		agent_detection_fill.rotation = angle;
		agent_detection_outline.rotation = angle;

		auto move_vec = glm::vec3(0.0f, 1.0f, 0.0f);
		move_vec = glm::rotateZ(move_vec, glm::radians(angle));

		agent_detection_outline.position = move_vec * enemy_data.radial_detection_offset + glm::vec3(0.0f, 0.0f, 0.1f);
		agent_detection_fill.position = move_vec * (enemy_data.radial_detection_offset + 2.0f);
	} else {
		agent_detection_fill.rotation = 0.0f;
		agent_detection_outline.rotation = 0.0f;
	}

	if (enemy_data.detection_level < 0.001f) {
		agent_detection_outline.display = false;
		agent_detection_fill.display = false;
	} else {
		agent_detection_outline.display = true;
		agent_detection_fill.display = true;
	}

	hacker_detection_outline.is_screen_space = true;
	hacker_detection_outline.is_billboard = false;

	hacker_detection_fill.is_screen_space = true;
	hacker_detection_fill.is_billboard = false;

	auto hacker_detection_pos = transform_to_screen(transform.get_global_position() + glm::vec3(0.0f, 2.0f, 0.0f), r_scene, true);
	hacker_detection_outline.position = glm::vec3(hacker_detection_pos,0.1f);
	hacker_detection_fill.position = glm::vec3(hacker_detection_pos + glm::vec2(0.0f, 1.0f),0.0f);

	hacker_detection_outline.size = enemy_data.detection_slider_default_size / 2.5f + enemy_data.detection_slider_default_size / (distance_to_hacker);
	hacker_detection_fill.size = glm::lerp(glm::vec2(0.0f), hacker_detection_outline.size * 0.95f, enemy_data.detection_level);

	if (enemy_data.detection_level < 0.4f) {
		hacker_detection_fill.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		hacker_detection_outline.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	} else if (enemy_data.detection_level >= 0.4f) {
		hacker_detection_fill.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		hacker_detection_outline.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}

	if (enemy_data.detection_level >= 0.99f) {
		hacker_detection_fill.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		hacker_detection_outline.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	auto hacker_pos_no_y = hacker_pos;
	hacker_pos_no_y.y = 0.0f;
	target_pos_no_y = transform.get_global_position();
	target_pos_no_y.y = 0.0f;
	dir = glm::normalize(hacker_pos_no_y - target_pos_no_y);
	cam_forward = scene->world.get_component<Transform>(GameplayManager::get().get_hacker_camera(scene)).get_global_forward();
	cam_forward_xz_proj = glm::normalize(glm::vec3(cam_forward.x, 0.0f, cam_forward.z));

	angle = glm::degrees(glm::acos(glm::dot(dir, cam_forward_xz_proj)));

	if (
			hacker_detection_outline.position.x > window_size.x / 2.0f ||
			hacker_detection_outline.position.y > window_size.y / 2.0f ||
			hacker_detection_outline.position.x < -window_size.x / 2.0f ||
			hacker_detection_outline.position.y < -window_size.y / 2.0f ||
			angle > 90.0f
			) {
		auto cross = glm::cross(dir, cam_forward_xz_proj);
		if (cross.y > 0.0f) {
			angle = -angle;
		}
		hacker_detection_fill.rotation = angle;
		hacker_detection_outline.rotation = angle;

		auto move_vec = glm::vec3(0.0f, 1.0f, 0.0f);
		move_vec = glm::rotateZ(move_vec, glm::radians(angle));

		hacker_detection_outline.position = move_vec * enemy_data.radial_detection_offset + glm::vec3(0.0f, 0.0f, 0.1f);
		hacker_detection_fill.position = move_vec * (enemy_data.radial_detection_offset + 2.0f);
	} else {
		hacker_detection_fill.rotation = 0.0f;
		hacker_detection_outline.rotation = 0.0f;
	}

	if (enemy_data.detection_level < 0.001f) {
		hacker_detection_outline.display = false;
		hacker_detection_fill.display = false;
	} else {
		hacker_detection_outline.display = true;
		hacker_detection_fill.display = true;
	}
}

inline void update_detection_slider_camera(
		uint32_t entity_id, Transform &transform, DetectionCamera &detection_camera, RenderScene &r_scene, Scene *scene) {
	//auto &agent_slider = UIManager::get().get_ui_slider(std::to_string(entity_id) + "_detection", "agent_detection_slider");
	//auto &hacker_slider = UIManager::get().get_ui_slider(std::to_string(entity_id) + "_detection", "hacker_detection_slider");

	auto &agent_detection_outline = UIManager::get().get_ui_image(std::to_string(entity_id) + "_detection", "agent_detection_outline");
	auto &hacker_detection_outline = UIManager::get().get_ui_image(std::to_string(entity_id) + "_detection", "hacker_detection_outline");
	auto &agent_detection_fill = UIManager::get().get_ui_image(std::to_string(entity_id) + "_detection", "agent_detection_fill");
	auto &hacker_detection_fill = UIManager::get().get_ui_image(std::to_string(entity_id) + "_detection", "hacker_detection_fill");

	auto &window_size = r_scene.render_extent;
	auto agent_pos = GameplayManager::get().get_agent_position(scene);
	auto hacker_pos = GameplayManager::get().get_hacker_position(scene);

	auto distance_to_agent = glm::clamp(glm::distance(transform.get_global_position(), agent_pos), 1.0f, 100.0f);
	auto distance_to_hacker = glm::clamp(glm::distance(transform.get_global_position(), hacker_pos), 1.0f, 100.0f);

	agent_detection_outline.is_screen_space = true;
	agent_detection_outline.is_billboard = false;

	agent_detection_fill.is_screen_space = true;
	agent_detection_fill.is_billboard = false;

	auto agent_detection_pos = transform_to_screen(transform.get_global_position() + glm::vec3(0.0f, 0.5f, 0.0f), r_scene, false);
	agent_detection_outline.position = glm::vec3(agent_detection_pos,0.1f);
	agent_detection_fill.position = glm::vec3(agent_detection_pos + glm::vec2(0.0f, 1.0f),0.0f);

	agent_detection_outline.size = detection_camera.default_detection_slider_size / 2.5f + detection_camera.default_detection_slider_size / (distance_to_agent);
	agent_detection_fill.size = glm::lerp(glm::vec2(0.0f), agent_detection_outline.size * 0.95f, detection_camera.detection_level);
	if (detection_camera.detection_level < 0.5f) {
		// todo sound
		agent_detection_fill.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		agent_detection_outline.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	} else if (detection_camera.detection_level >= 0.5f) {
		agent_detection_fill.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		agent_detection_outline.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}

	if (detection_camera.detection_level >= 0.99f) {
		agent_detection_fill.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		agent_detection_outline.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	auto agent_pos_no_y = agent_pos;
	agent_pos_no_y.y = 0.0f;
	auto target_pos_no_y = transform.get_global_position();
	target_pos_no_y.y = 0.0f;
	auto dir = glm::normalize(agent_pos_no_y - target_pos_no_y);
	auto cam_forward = scene->world.get_component<Transform>(GameplayManager::get().get_agent_camera(scene)).get_global_forward();
	auto cam_forward_xz_proj = glm::normalize(glm::vec3(cam_forward.x, 0.0f, cam_forward.z));

	auto angle = glm::degrees(glm::acos(glm::dot(dir, cam_forward_xz_proj)));

	if (
			agent_detection_outline.position.x > window_size.x / 2.0f ||
			agent_detection_outline.position.y > window_size.y / 2.0f ||
			agent_detection_outline.position.x < -window_size.x / 2.0f ||
			agent_detection_outline.position.y < -window_size.y / 2.0f ||
			angle > 90.0f
		) {
		auto cross = glm::cross(dir, cam_forward_xz_proj);
		if (cross.y > 0.0f) {
			angle = -angle;
		}
		agent_detection_fill.rotation = angle;
		agent_detection_outline.rotation = angle;

		auto move_vec = glm::vec3(0.0f, 1.0f, 0.0f);
		move_vec = glm::rotateZ(move_vec, glm::radians(angle));

		agent_detection_outline.position = move_vec * detection_camera.radial_detection_offset + glm::vec3(0.0f, 0.0f, 0.1f);
		agent_detection_fill.position = move_vec * (detection_camera.radial_detection_offset + 2.0f);
	} else {
		agent_detection_fill.rotation = 0.0f;
		agent_detection_outline.rotation = 0.0f;
	}

	if (detection_camera.detection_level < 0.001f) {
		agent_detection_outline.display = false;
		agent_detection_fill.display = false;
	} else {
		agent_detection_outline.display = true;
		agent_detection_fill.display = true;
	}

	hacker_detection_outline.is_screen_space = true;
	hacker_detection_outline.is_billboard = false;

	hacker_detection_fill.is_screen_space = true;
	hacker_detection_fill.is_billboard = false;

	hacker_detection_outline.size = detection_camera.default_detection_slider_size / 2.5f + detection_camera.default_detection_slider_size / (distance_to_hacker);
	hacker_detection_fill.size = glm::lerp(glm::vec2(0.0f), hacker_detection_outline.size * 0.95f, detection_camera.detection_level);
	if (detection_camera.detection_level < 0.5f) {
		hacker_detection_fill.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		hacker_detection_outline.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
	} else if (detection_camera.detection_level >= 0.5f) {
		hacker_detection_fill.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
		hacker_detection_outline.color = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
	if (detection_camera.detection_level >= 0.99f) {
		hacker_detection_fill.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		hacker_detection_outline.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	}

	auto hacker_detection_pos = transform_to_screen(transform.get_global_position() + glm::vec3(0.0f, 0.5f, 0.0f), r_scene, true);
	hacker_detection_outline.position = glm::vec3(hacker_detection_pos,0.1f);
	hacker_detection_fill.position = glm::vec3(hacker_detection_pos + glm::vec2(0.0f, 1.0f),0.0f);

	auto hacker_pos_no_y = hacker_pos;
	hacker_pos_no_y.y = 0.0f;
	target_pos_no_y = transform.get_global_position();
	target_pos_no_y.y = 0.0f;
	dir = glm::normalize(hacker_pos_no_y - target_pos_no_y);
	cam_forward = scene->world.get_component<Transform>(GameplayManager::get().get_hacker_camera(scene)).get_global_forward();
	cam_forward_xz_proj = glm::normalize(glm::vec3(cam_forward.x, 0.0f, cam_forward.z));

	angle = glm::degrees(glm::acos(glm::dot(dir, cam_forward_xz_proj)));

	if (
			hacker_detection_outline.position.x > window_size.x / 2.0f ||
			hacker_detection_outline.position.y > window_size.y / 2.0f ||
			hacker_detection_outline.position.x < -window_size.x / 2.0f ||
			hacker_detection_outline.position.y < -window_size.y / 2.0f ||
			angle > 90.0f
	) {
		auto cross = glm::cross(dir, cam_forward_xz_proj);
		if (cross.y > 0.0f) {
			angle = -angle;
		}

		hacker_detection_fill.rotation = angle;
		hacker_detection_outline.rotation = angle;

		auto move_vec = glm::vec3(0.0f, 1.0f, 0.0f);
		move_vec = glm::rotateZ(move_vec, glm::radians(angle));

		hacker_detection_outline.position = move_vec * detection_camera.radial_detection_offset + glm::vec3(0.0f, 0.0f, 0.1f);
		hacker_detection_fill.position = move_vec * (detection_camera.radial_detection_offset + 2.0f);
	} else {
		hacker_detection_fill.rotation = 0.0f;
		hacker_detection_outline.rotation = 0.0f;
	}

	if (detection_camera.detection_level < 0.001f) {
		hacker_detection_outline.display = false;
		hacker_detection_fill.display = false;
	} else {
		hacker_detection_outline.display = true;
		hacker_detection_fill.display = true;
	}

	if (detection_camera.detection_level > 0.01f) {
		std::cout << "outline (display): " << hacker_detection_outline.display << ", fill (display): " << hacker_detection_fill.display << std::endl;
		std::cout << "outline (position): " << glm::to_string(hacker_detection_outline.position) << ", fill (position): " << glm::to_string(hacker_detection_fill.position) << std::endl;
		std::cout << "outline (size): " << glm::to_string(hacker_detection_outline.size) << ", fill (size): " << glm::to_string(hacker_detection_fill.size) << std::endl;
		std::cout << "outline (color): " << glm::to_string(hacker_detection_outline.color) << ", fill (color): " << glm::to_string(hacker_detection_fill.color) << std::endl;
	}
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

inline void handle_footsteps(uint32_t entity, Transform &transform, EnemyData &enemy_data, float dt) {
	auto &audio = AudioManager::get();
	auto &animation_manager = AnimationManager::get();

	const glm::mat4 &left_foot_bone_matrix = animation_manager.get_bone_transform(entity, left_foot_bone);
	const glm::mat4 &right_foot_bone_matrix = animation_manager.get_bone_transform(entity, right_foot_bone);

	// get positions from both matrices
	auto left_foot_position = glm::vec3(left_foot_bone_matrix[3]);
	auto right_foot_position = glm::vec3(right_foot_bone_matrix[3]);
	if (right_foot_position.y < footstep_right_down_threshold && enemy_data.right_foot_can_play) {
		audio.play_one_shot_3d(enemy_data.footsteps_event, transform);
		enemy_data.right_foot_can_play = false;
	} else if (left_foot_position.y < footstep_left_down_threshold && enemy_data.left_foot_can_play) {
		audio.play_one_shot_3d(enemy_data.footsteps_event, transform);
		enemy_data.left_foot_can_play = false;
	} else {
		if (left_foot_position.y > footstep_up_threshold) {
			enemy_data.left_foot_can_play = true;
		}
		if (right_foot_position.y > footstep_up_threshold) {
			enemy_data.right_foot_can_play = true;
		}
	}
}

} //namespace enemy_utils

#endif //SILENCE_ENEMY_UTILS_H
