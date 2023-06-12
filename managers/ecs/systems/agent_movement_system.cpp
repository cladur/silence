#include "agent_movement_system.h"
#include "components/agent_data_component.h"
#include "components/collider_capsule.h"
#include "components/enemy_data_component.h"
#include "components/platform_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"

#include "animation/animation_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "physics/physics_manager.h"

#include "input/input_manager.h"
#include "resource/resource_manager.h"
#include <audio/audio_manager.h>
#include <gameplay/gameplay_manager.h>
#include <render/transparent_elements/ui_manager.h>
#include <spdlog/spdlog.h>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtx/string_cast.hpp>

AutoCVarFloat cvar_agent_acc_ground("agent.acc_ground", "acceleration on ground", 32.0f, CVarFlags::EditFloatDrag);

AutoCVarFloat cvar_agent_max_vel_ground(
		"agent.max_vel_ground", "maximum velocity on ground", 4.0f, CVarFlags::EditFloatDrag);

AutoCVarFloat cvar_friction_ground("agent.friction_ground", "friction on ground", 9.0f, CVarFlags::EditFloatDrag);

AutoCVarFloat cvar_agent_crouch_slowdown(
		"agent.crouch_slowdown", "slowdown while crouching", 0.6f, CVarFlags::EditFloatDrag);

// AutoCVarFloat cvar_floor_len("slope.floor_len", "slowdown while crouching", 0.1f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_max_snap_length("slope.max_snap_length",
		"maximum distance from the ground, that won't make the character fall", 0.3f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_too_steep(
		"slope.too_steep", "angle at which the character will slide off the slope", 60.0f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_snap_offset("slope.snap_offset", "how high to snap the player over the floor, if he's close to it",
		0.2f, CVarFlags::EditFloatDrag);

void AgentMovementSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<AgentData>());

	world.set_system_component_whitelist<AgentMovementSystem>(whitelist);
}

void AgentMovementSystem::update(World &world, float dt) {
	ZoneScopedN("AgentMovementSystem::update");
	InputManager &input_manager = InputManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();

	static glm::vec3 previous_velocity = { 0, 0, 0 };

	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &agent_data = world.get_component<AgentData>(entity);
		auto &capsule_collider = world.get_component<ColliderCapsule>(entity);
		auto &camera_pivot_tf = world.get_component<Transform>(agent_data.camera_pivot);
		auto &model_tf = world.get_component<Transform>(agent_data.model);
		auto &camera_tf = world.get_component<Transform>(agent_data.camera);
		auto &spring_arm_tf = world.get_component<Transform>(agent_data.spring_arm);
		auto &animation_instance = world.get_component<AnimationInstance>(agent_data.model);
		auto &camera = world.get_component<Camera>(agent_data.camera);

		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		auto &is_crouching = agent_data.is_crouching;

		if (agent_data.locked_movement) {
			previous_velocity = {0.0f, 0.0f, 0.0f};
			continue;
		}

		auto camera_forward = camera_pivot_tf.get_global_forward();

		camera_forward.y = 0.0f;
		camera_forward = glm::normalize(camera_forward);
		auto camera_right = camera_pivot_tf.get_global_right();

		glm::vec3 acc_direction = { 0, 0, 0 };
		acc_direction += input_manager.get_axis("agent_move_backward", "agent_move_forward") * camera_forward;
		acc_direction += input_manager.get_axis("agent_move_left", "agent_move_right") * -camera_right;

		if (glm::length(acc_direction) > 1) {
			acc_direction = glm::normalize(acc_direction);
		}

		float acceleration = cvar_agent_acc_ground.get();
		if (is_crouching) {
			acceleration *= cvar_agent_crouch_slowdown.get();
		}

		glm::vec3 velocity = move_ground(acc_direction, previous_velocity, acceleration, dt);

		//SPDLOG_INFO(animation_instance.ticks_per_second);
		transform.add_position(glm::vec3(velocity.x, 0.0, velocity.z) * dt);

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
		ray.origin = transform.get_global_position() + glm::vec3(0.0f, 1.0f, 0.0f);
		ray.ignore_list.emplace_back(entity);
		ray.layer_name = "default";
		ray.direction = -transform.get_up();
		glm::vec3 end = ray.origin + ray.direction;
		HitInfo info;
		if (CollisionSystem::ray_cast_layer(world, ray, info)) {
			// If the agent is not on the ground, move him down
			// If the agent is on 40 degree slope or more, move him down
			bool is_on_too_steep_slope =
					glm::dot(info.normal, glm::vec3(0.0f, 1.0f, 0.0f)) < glm::cos(glm::radians(cvar_too_steep.get()));

			bool under_floor = info.distance < 1.0f;
			bool in_snappable_range = info.distance < 1.0f + cvar_max_snap_length.get();

			// If the agent is close enough to the floor, snap him to it
			if (in_snappable_range || under_floor) {
				transform.position.y = info.point.y + cvar_snap_offset.get();
				transform.set_changed(true);
			} else if (is_on_too_steep_slope || info.distance > 1.01f) {
				transform.position.y -= 8.0f * dt;
				transform.set_changed(true);
			}
			if (world.has_component<Platform>(info.entity)) {
				auto &platform = world.get_component<Platform>(info.entity);
				if (platform.is_moving) {
					transform.add_position(platform.change_vector);
				}
			}
		}

		last_position = transform.position;
		previous_velocity = velocity;
	}
}

glm::vec3 AgentMovementSystem::accelerate(
		glm::vec3 accel_dir, glm::vec3 prev_velocity, float acceleration, float max_velocity, float dt) {
	float proj_vel = glm::dot(prev_velocity, accel_dir);
	float accel_vel = acceleration * dt;

	if (proj_vel + accel_vel > max_velocity) {
		accel_vel = max_velocity - proj_vel;
	}

	return prev_velocity + accel_dir * accel_vel;
}

glm::vec3 AgentMovementSystem::move_ground(glm::vec3 accel_dir, glm::vec3 pre_velocity, float acceleration, float dt) {
	float speed = glm::length(pre_velocity);
	if (speed != 0) {
		float drop = speed * cvar_friction_ground.get() * dt;
		pre_velocity *= glm::max(speed - drop, 0.0f) / speed;
	}

	return accelerate(accel_dir, pre_velocity, acceleration, cvar_agent_max_vel_ground.get(), dt);
}