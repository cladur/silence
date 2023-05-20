#include "hacker_system.h"
#include "components/hacker_data_component.h"
#include "components/interactable_component.h"
#include "components/light_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"

#include "ecs/systems/collision_system.h"
#include "physics/physics_manager.h"

#include "input/input_manager.h"
#include "resource/resource_manager.h"
#include <spdlog/spdlog.h>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

AutoCVarFloat cvar_hacker_acc_ground("hacker.acc_ground", "acceleration on ground ", 0.4f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_max_vel_ground(
		"hacker.max_vel_ground", "maximum velocity on ground ", 2.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_friction_ground(
		"hacker.friction_ground", "friction on ground", 8.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_camera_sensitivity(
		"settings.hacker_camera_sensitivity", "camera sensitivity", 0.1f, CVarFlags::EditCheckbox);

bool shoot_raycast(Transform &transform, World &world, float dt, glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f)) {
	Ray ray;
	ray.origin = transform.get_global_position() + glm::vec3(0.0f, 0.0f, -1.0f);
	ray.direction = direction;
	ray.layer_name = "hacker";
	glm::vec3 end = ray.origin + direction * 100.0f;
	world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(ray.origin, end, glm::vec3(1.0f, 0.0f, 0.0f));
	HitInfo info;
	bool hit = CollisionSystem::ray_cast_layer(world, ray, info);

	if (hit) {
		Entity hit_entity = info.entity;
		if (info.entity != 0) {
			if (world.has_component<Interactable>(hit_entity)) {
				auto &interactable = world.get_component<Interactable>(hit_entity);

				if (!(interactable.type == InteractionType::Hacker)) {
					SPDLOG_WARN("Entity {} is not a hacker interactable", hit_entity);
					return false;
				}

				if (!interactable.can_interact) {
					SPDLOG_WARN("Entity {} cannot be interacted with", hit_entity);
					return false;
				}

				interactable.triggered = true;
				return true;
			}
		}
	} else {
		SPDLOG_ERROR("NO HIT");
		return false;
	}
}

void HackerSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<HackerData>());

	world.set_system_component_whitelist<HackerSystem>(whitelist);

	previous_velocity = { 0, 0, 0 };
}

void HackerSystem::update(World &world, float dt) {
	InputManager &input_manager = InputManager::get();
	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &hacker_data = world.get_component<HackerData>(entity);
		auto &camera_pivot_tf = world.get_component<Transform>(hacker_data.camera_pivot);
		auto &model_tf = world.get_component<Transform>(hacker_data.model);
		auto &camera_tf = world.get_component<Transform>(hacker_data.camera);
		auto &animation_instance = world.get_component<AnimationInstance>(hacker_data.model);

		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		auto camera_forward = camera_pivot_tf.get_global_forward();
		glm::vec3 real_camera_forward = camera_forward;

		camera_forward.y = 0.0f;
		camera_forward = glm::normalize(camera_forward);
		auto camera_right = camera_pivot_tf.get_global_right();

		world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(
				camera_pivot_tf.get_global_position() + glm::vec3(0.0f, 0.0f, -1.0f),
				camera_pivot_tf.get_global_position() + glm::vec3(0.0f, 0.0f, -1.0f) + real_camera_forward * 100.0f,
				glm::vec3(1.0f, 0.0f, 0.0f));

		glm::vec3 acc_direction = { 0, 0, 0 };
		if (input_manager.is_action_pressed("move_forward")) {
			acc_direction += camera_forward;
		}
		if (input_manager.is_action_pressed("move_backward")) {
			acc_direction -= camera_forward;
		}
		if (input_manager.is_action_pressed("move_left")) {
			acc_direction += camera_right;
		}
		if (input_manager.is_action_pressed("move_right")) {
			acc_direction -= camera_right;
		}
		if (input_manager.is_action_just_pressed("mouse_left")) {
			shoot_raycast(camera_pivot_tf, world, dt, real_camera_forward);
		}
		glm::normalize(acc_direction);
		glm::vec3 velocity = move_ground(acc_direction, previous_velocity, dt);
		if (glm::length(velocity) > 0.001f) {
			animation_instance.animation_handle = ResourceManager::get().get_animation_handle(
					"scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_walk.anim");
		} else {
			animation_instance.animation_handle = ResourceManager::get().get_animation_handle(
					"scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_idle.anim");
		}

		transform.add_position(glm::vec3(velocity.x, 0.0, velocity.z));

		glm::vec2 mouse_delta = input_manager.get_mouse_delta();
		camera_pivot_tf.add_euler_rot(glm::vec3(mouse_delta.y, 0.0f, 0.0f) * cvar_hacker_camera_sensitivity.get() * dt);
		camera_pivot_tf.add_global_euler_rot(
				glm::vec3(0.0f, -mouse_delta.x, 0.0f) * cvar_hacker_camera_sensitivity.get() * dt);

		static glm::vec3 last_position = transform.position;

		// Lerp model_tf towards movement direction
		glm::vec3 delta_position = transform.position - last_position;
		delta_position.y = 0.0f;
		if (glm::length2(delta_position) > 0.0001f) {
			glm::vec3 direction = glm::normalize(delta_position);
			glm::vec3 forward =
					glm::normalize(glm::vec3(model_tf.get_global_forward().x, 0.0f, model_tf.get_global_forward().z));
			float angle = glm::acos(glm::dot(forward, direction)) * 0.3f;
			glm::vec3 axis = glm::cross(forward, direction);
			model_tf.add_global_euler_rot(axis * angle);
		}

		Ray ray{};
		ray.origin = transform.get_global_position() + glm::vec3(0.0f, -0.01f, 0.0f);
		ray.direction = glm::vec3(0.0f, -1.0f, 0.0f);
		glm::vec3 end = ray.origin + ray.direction;
		world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(ray.origin, end);
		HitInfo info;
		if (CollisionSystem::ray_cast(world, ray, info)) {
			// If the hacker is not on the ground, move him down
			// If the hacker is on 40 degree slope or more, move him down
			bool is_on_too_steep_slope =
					glm::dot(info.normal, glm::vec3(0.0f, 1.0f, 0.0f)) < glm::cos(glm::radians(40.0f));

			if (is_on_too_steep_slope || info.distance > 0.1f) {
				transform.position.y -= 8.0f * dt;
				transform.set_changed(true);
			}

			// If the hacker is close enough to the floor, snap him to it
			float snap_length = 0.3f;
			if (info.distance > 0.1f && info.distance < snap_length) {
				transform.position.y = info.point.y + 0.001f;
				transform.set_changed(true);
			}
		}

		last_position = transform.position;
		previous_velocity = velocity;
	}
}

glm::vec3 HackerSystem::accelerate(
		glm::vec3 accel_dir, glm::vec3 prev_velocity, float acceleration, float max_velocity, float dt) {
	float proj_vel = glm::dot(prev_velocity, accel_dir);
	float accel_vel = acceleration * dt;

	if (proj_vel + accel_vel > max_velocity) {
		accel_vel = max_velocity - proj_vel;
	}

	return prev_velocity + accel_dir * accel_vel;
}

glm::vec3 HackerSystem::move_ground(glm::vec3 accel_dir, glm::vec3 pre_velocity, float dt) {
	float speed = glm::length(pre_velocity);
	if (speed != 0) {
		float drop = speed * cvar_hacker_friction_ground.get() * dt;
		pre_velocity *= glm::max(speed - drop, 0.0f) / speed;
	}

	return accelerate(accel_dir, pre_velocity, cvar_hacker_acc_ground.get(), cvar_hacker_max_vel_ground.get(), dt);
}