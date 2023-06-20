#include "ecs/systems/hacker_movement_system.h"
#include "animation/animation_manager.h"
#include "components/hacker_data_component.h"
#include "ecs/world.h"
#include "input/input_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "physics/physics_manager.h"
#include <audio/audio_manager.h>
#include <gameplay/gameplay_manager.h>

AutoCVarFloat cvar_hacker_acc_ground("hacker.acc_ground", "acceleration on ground ", 0.4f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_minimal_animation_speed(
		"hacker.minimal_animation_speed", "minimal walking animation speed ", 3500.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_velocity_animation_speed_multiplier("hacker.velocity_animation_speed_multiplier",
		"glm::length(velocity) * this", 400000.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_idle_animation_tickrate(
		"hacker.idle_animation_tickrate", "Idle animation tickrate", 1500.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_max_vel_ground(
		"hacker.max_vel_ground", "maximum velocity on ground ", 2.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_friction_ground(
		"hacker.friction_ground", "friction on ground", 8.0f, CVarFlags::EditCheckbox);

void HackerMovementSystem::startup(World &world) {
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<HackerData>());

	world.set_system_component_whitelist<HackerMovementSystem>(whitelist);

	step_event = EventReference("Hacker/step");
}

void HackerMovementSystem::update(World &world, float dt) {
	ZoneScopedN("HackerSystem::update");
	if (GameplayManager::get().game_state == GameState::MAIN_MENU) {
		return;
	}
	InputManager &input_manager = InputManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();

	for (const Entity entity : entities) {
		auto &hacker_data = world.get_component<HackerData>(entity);
		auto &transform = world.get_component<Transform>(entity);
		auto &camera_pivot_tf = world.get_component<Transform>(hacker_data.camera_pivot);
		auto &model_tf = world.get_component<Transform>(hacker_data.model);
		auto &camera_tf = world.get_component<Transform>(hacker_data.camera);
		auto &animation_instance = world.get_component<AnimationInstance>(hacker_data.model);

		bool is_on_camera = hacker_data.is_on_camera;
		glm::vec3 camera_forward = -camera_tf.get_global_forward();
		auto camera_right = camera_pivot_tf.get_global_right();

		glm::vec3 acc_direction = { 0, 0, 0 };
		if (!is_on_camera) {

			if(hacker_data.gamepad >= 0) {
			acc_direction += input_manager.get_axis("hacker_move_backward", "hacker_move_forward", hacker_data.gamepad) * camera_forward;
			acc_direction += input_manager.get_axis("hacker_move_left", "hacker_move_right", hacker_data.gamepad) * -camera_right;
			} else {
			acc_direction += input_manager.get_axis("hacker_move_backward", "hacker_move_forward") * camera_forward;
			acc_direction += input_manager.get_axis("hacker_move_left", "hacker_move_right") * -camera_right;
			}
		}

		if (*CVarSystem::get()->get_int_cvar("debug_camera.use")) {
			acc_direction = glm::vec3(0.0f);
		}

		if (glm::length(acc_direction) > 0.0f) {
			acc_direction = glm::normalize(acc_direction);
		}

		glm::vec3 velocity = move_ground(acc_direction, previous_velocity, dt);
		if (glm::dot(velocity, velocity) > physics_manager.get_epsilon()) {
			if (animation_instance.animation_handle.id !=
					resource_manager.get_animation_handle("scorpion/scorpion_ANIM_GLTF/scorpion_walk.anim").id) {
				animation_manager.change_animation(hacker_data.model, "scorpion/scorpion_ANIM_GLTF/scorpion_walk.anim");
			}

			if (glm::dot(velocity, velocity) > 0.001f) {
				if (current_time >= time_per_step) {
					AudioManager::get().play_one_shot_3d(step_event, transform);
					current_time = 0.0f;
				}
				current_time += dt;
			}
		} else {
			if (animation_instance.animation_handle.id !=
					resource_manager.get_animation_handle("scorpion/scorpion_ANIM_GLTF/scorpion_idle.anim").id) {
				animation_manager.change_animation(hacker_data.model, "scorpion/scorpion_ANIM_GLTF/scorpion_idle.anim");
			}
		}
		if (glm::length(velocity) == 0.0f) {
			animation_instance.ticks_per_second = cvar_hacker_idle_animation_tickrate.get();
		} else {
			float minimal_speed = cvar_hacker_minimal_animation_speed.get();
			float new_animation_speed = glm::length(velocity) * cvar_hacker_velocity_animation_speed_multiplier.get();
			if (new_animation_speed < minimal_speed) {
				new_animation_speed = minimal_speed;
			}
			animation_instance.ticks_per_second = new_animation_speed;
		}
		transform.add_position(glm::vec3(velocity.x, 0.0, velocity.z));

		static glm::vec3 last_position = transform.position;

		// Lerp model_tf towards movement direction
		glm::vec3 delta_position = transform.position - last_position;

		delta_position.y = 0.0f;
		if (glm::length2(delta_position) > 0.001f) {
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
			auto cvar_system = CVarSystem::get();
			// If the hacker is not on the ground, move him down
			// If the hacker is on 40 degree slope or more, move him down
			bool is_on_too_steep_slope = glm::dot(info.normal, glm::vec3(0.0f, 1.0f, 0.0f)) <
					glm::cos(glm::radians(*cvar_system->get_float_cvar("slope.too_steep")));

			bool under_floor = info.distance < 1.0f;
			bool in_snappable_range = info.distance < 1.0f + *cvar_system->get_float_cvar("slope.max_snap_length");

			// If the hacker is close enough to the floor, snap him to it
			if (in_snappable_range || under_floor) {
				transform.position.y = info.point.y + *cvar_system->get_float_cvar("slope.snap_offset");
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

		if (glm::length(velocity) < 0.0001f) {
			velocity = glm::vec3(0.0f);
		}
		last_position = transform.position;
		previous_velocity = velocity;
	}
}

glm::vec3 HackerMovementSystem::accelerate(
		glm::vec3 accel_dir, glm::vec3 prev_velocity, float acceleration, float max_velocity, float dt) {
	float proj_vel = glm::dot(prev_velocity, accel_dir);
	float accel_vel = acceleration * dt;

	if (proj_vel + accel_vel > max_velocity) {
		accel_vel = max_velocity - proj_vel;
	}

	return prev_velocity + accel_dir * accel_vel;
}

glm::vec3 HackerMovementSystem::move_ground(glm::vec3 accel_dir, glm::vec3 pre_velocity, float dt) {
	float speed = glm::length(pre_velocity);
	if (speed != 0) {
		float drop = speed * cvar_hacker_friction_ground.get() * dt;
		pre_velocity *= glm::max(speed - drop, 0.0f) / speed;
	}

	return accelerate(accel_dir, pre_velocity, cvar_hacker_acc_ground.get(), cvar_hacker_max_vel_ground.get(), dt);
}