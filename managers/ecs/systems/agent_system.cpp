#include "agent_system.h"
#include "components/agent_data_component.h"
#include "components/collider_capsule.h"
#include "components/platform_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"

#include "animation/animation_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "physics/physics_manager.h"

#include "input/input_manager.h"
#include "resource/resource_manager.h"
#include <spdlog/spdlog.h>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

AutoCVarFloat cvar_agent_interaction_range(
		"agent.interaction_range", "range of interaction", 1.5f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_agent_acc_ground("agent.acc_ground", "acceleration on ground", 0.4f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_agent_crouch_slowdown(
		"agent.crouch_slowdown", "slowdown while crouching", 2.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_agent_max_vel_ground(
		"agent.max_vel_ground", "maximum velocity on ground", 2.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_friction_ground("agent.friction_ground", "friction on ground", 8.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_camera_sensitivity(
		"settings.camera_sensitivity", "camera sensitivity", 0.1f, CVarFlags::EditCheckbox);

void AgentSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<AgentData>());

	world.set_system_component_whitelist<AgentSystem>(whitelist);

	previous_velocity = { 0, 0, 0 };
}

void AgentSystem::update(World &world, float dt) {
	InputManager &input_manager = InputManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();
	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &agent_data = world.get_component<AgentData>(entity);
		auto &capsule_collider= world.get_component<ColliderCapsule>(entity);
		auto &camera_pivot_tf = world.get_component<Transform>(agent_data.camera_pivot);
		auto &model_tf = world.get_component<Transform>(agent_data.model);
		auto &camera_tf = world.get_component<Transform>(agent_data.camera);
		auto &animation_instance = world.get_component<AnimationInstance>(agent_data.model);

		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		auto camera_forward = camera_pivot_tf.get_global_forward();

		camera_forward.y = 0.0f;
		camera_forward = glm::normalize(camera_forward);
		auto camera_right = camera_pivot_tf.get_global_right();

		glm::vec3 acc_direction = { 0, 0, 0 };
		if (input_manager.is_action_pressed("agent_move_forward")) {
			acc_direction += camera_forward;
		}
		if (input_manager.is_action_pressed("agent_move_backward")) {
			acc_direction -= camera_forward;
		}
		if (input_manager.is_action_pressed("agent_move_left")) {
			acc_direction += camera_right;
		}
		if (input_manager.is_action_pressed("agent_move_right")) {
			acc_direction -= camera_right;
		}
		glm::normalize(acc_direction);


		//TODO: replace hard coded values with one derived from collider
		if (input_manager.is_action_just_pressed("agent_crouch")) {
			if(!is_crouching) {
				is_crouching = true;
				capsule_collider.end.y = 0.85f;
			}
			else {
				Ray ray{};
				ray.origin = transform.get_global_position() + glm::vec3(0.0f, 1.0f, 0.0f);
				ray.direction = model_tf.get_global_up();
				ray.ignore_list.emplace_back(entity);
				ray.layer_name = "agent";
				glm::vec3 end = ray.origin + ray.direction;
				HitInfo info;
				bool hit = CollisionSystem::ray_cast_layer(world, ray, info);
				SPDLOG_INFO(hit);
				if(!hit || info.distance > 0.8f){
					SPDLOG_INFO(info.distance);
					capsule_collider.end.y = 1.3f;
					is_crouching = false;
				}			
			}		
		}

		if (*CVarSystem::get()->get_int_cvar("debug_camera.use")) {
			acc_direction = glm::vec3(0.0f, 0.0f, 0.0f);
		}

		glm::vec3 velocity = move_ground(acc_direction, previous_velocity, dt);

		// Use dot instead of length when u want to check 0 value, to avoid calculating square root
		if (glm::dot(velocity, velocity) > physics_manager.get_epsilon()) {
			if (is_crouching) {
				if (animation_instance.animation_handle.id !=
						resource_manager.get_animation_handle("agent/agent_ANIM_GLTF/agent_crouch.anim").id) {
					animation_manager.change_animation(agent_data.model, "agent/agent_ANIM_GLTF/agent_crouch.anim");
				}
			} else {
				if (animation_instance.animation_handle.id !=
						resource_manager.get_animation_handle("agent/agent_ANIM_GLTF/agent_walk_stealthy.anim").id) {
					animation_manager.change_animation(
							agent_data.model, "agent/agent_ANIM_GLTF/agent_walk_stealthy.anim");
				}
			}

		} else if (animation_timer <= 0.0f) {
			if (is_crouching) {
				if (animation_instance.animation_handle.id !=
						resource_manager.get_animation_handle("agent/agent_ANIM_GLTF/agent_crouch_idle.anim").id) {
					animation_manager.change_animation(
							agent_data.model, "agent/agent_ANIM_GLTF/agent_crouch_idle.anim");
				}
			} else {
				if (animation_instance.animation_handle.id !=
						resource_manager.get_animation_handle("agent/agent_ANIM_GLTF/agent_idle.anim").id) {
					animation_manager.change_animation(agent_data.model, "agent/agent_ANIM_GLTF/agent_idle.anim");
				}
			}
		}

		transform.add_position(glm::vec3(velocity.x, 0.0, velocity.z));

		if (*CVarSystem::get()->get_int_cvar("game.controlling_agent") &&
				!*CVarSystem::get()->get_int_cvar("debug_camera.use")) {
			glm::vec2 mouse_delta = input_manager.get_mouse_delta();
			camera_pivot_tf.add_euler_rot(glm::vec3(mouse_delta.y, 0.0f, 0.0f) * cvar_camera_sensitivity.get() * dt);
			camera_pivot_tf.add_global_euler_rot(
					glm::vec3(0.0f, -mouse_delta.x, 0.0f) * cvar_camera_sensitivity.get() * dt);
		}
		static glm::vec3 last_position = transform.position;

		// Lerp model_tf towards movement direction
		glm::vec3 delta_position = transform.position - last_position;
		delta_position.y = 0.0f;
		if (glm::length2(delta_position) > physics_manager.get_epsilon()) {
			glm::vec3 direction = glm::normalize(delta_position);
			glm::vec3 forward =
					glm::normalize(glm::vec3(model_tf.get_global_forward().x, 0.0f, model_tf.get_global_forward().z));
			float angle = glm::acos(glm::clamp(glm::dot(forward, direction), -1.0f, 1.0f)) * 0.3f;
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
			// If the agent is not on the ground, move him down
			// If the agent is on 40 degree slope or more, move him down
			bool is_on_too_steep_slope =
					glm::dot(info.normal, glm::vec3(0.0f, 1.0f, 0.0f)) < glm::cos(glm::radians(40.0f));

			if (is_on_too_steep_slope || info.distance > 0.1f) {
				transform.position.y -= 8.0f * dt;
				transform.set_changed(true);
			}
			else if (world.has_component<Platform>(info.entity)) {
				auto &platform = world.get_component<Platform>(info.entity);
				if(platform.is_moving) {
					transform.add_position(platform.change_vector);
				}
			}

			// If the agent is close enough to the floor, snap him to it
			float snap_length = 0.3f;
			if (info.distance > 0.1f && info.distance < snap_length) {
				transform.position.y = info.point.y + 0.001f;
				transform.set_changed(true);
			}
		}

		if (input_manager.is_action_just_pressed("agent_interact")) {
			Ray ray{};
			ray.origin = transform.get_global_position() + glm::vec3(0.0f, 1.0f, 0.0f);
			ray.direction = model_tf.get_global_forward();
			ray.ignore_list.emplace_back(entity);
			ray.layer_name = "agent";
			glm::vec3 end = ray.origin + ray.direction;
			world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(ray.origin, end, { 1.0f, 0.0f, 0.0f });
			HitInfo info;
			if (CollisionSystem::ray_cast_layer(world, ray, info)) {
				//TODO: remove log
				// auto hit_name = world.get_component<Name>(info.entity);
				// SPDLOG_INFO(hit_name.name);
				auto &target_transform = world.get_component<Transform>(info.entity);
				if (info.distance < cvar_agent_interaction_range.get()) {
					if (world.has_component<Interactable>(info.entity)) {
						auto &interactable = world.get_component<Interactable>(info.entity);
						if ((interactable.type == InteractionType::Agent) && interactable.can_interact) {
							interactable.triggered = true;
							auto animation_handle = resource_manager.get_animation_handle(
									"agent/agent_ANIM_GLTF/agent_interaction.anim");
							if (animation_instance.animation_handle.id != animation_handle.id) {
								animation_timer = resource_manager.get_animation(animation_handle).get_duration();
								previous_velocity = { 0.0f, 0.0f, 0.0f };
								velocity = { 0.0f, 0.0f, 0.0f };
								animation_manager.change_animation(
										agent_data.model, "agent/agent_ANIM_GLTF/agent_interaction.anim");
							}
						}
					}
				}
			}
		}

		if (animation_timer > 0) {
			animation_timer -= (dt * 1000);
		}
		last_position = transform.position;
		previous_velocity = velocity;
	}
}

glm::vec3 AgentSystem::accelerate(
		glm::vec3 accel_dir, glm::vec3 prev_velocity, float acceleration, float max_velocity, float dt) {
	float proj_vel = glm::dot(prev_velocity, accel_dir);
	float accel_vel = acceleration * dt;

	if (proj_vel + accel_vel > max_velocity) {
		accel_vel = max_velocity - proj_vel;
	}

	return prev_velocity + accel_dir * accel_vel;
}

glm::vec3 AgentSystem::move_ground(glm::vec3 accel_dir, glm::vec3 pre_velocity, float dt) {
	float speed = glm::length(pre_velocity);
	if (speed != 0) {
		float drop = speed * cvar_friction_ground.get() * dt;
		pre_velocity *= glm::max(speed - drop, 0.0f) / speed;
	}
	float acceleration = 0;
	if (is_crouching) {
		acceleration = cvar_agent_acc_ground.get() / cvar_agent_crouch_slowdown.get();
	} else {
		acceleration = cvar_agent_acc_ground.get();
	}

	return accelerate(accel_dir, pre_velocity, acceleration, cvar_agent_max_vel_ground.get(), dt);
}