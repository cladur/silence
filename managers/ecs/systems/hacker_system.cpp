#include "hacker_system.h"
#include "components/interactable_component.h"
#include "components/light_component.h"
#include "components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"

#include "animation/animation_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "physics/physics_manager.h"

#include "input/input_manager.h"
#include "resource/resource_manager.h"
#include <render/transparent_elements/ui_manager.h>
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

AutoCVarInt cvar_hacker_on_keyboard(
		"settings.hacker_on_keyboard", "Control hacker with keyboard + mouse", 0, CVarFlags::EditCheckbox);

bool HackerSystem::shoot_raycast(
		Transform &transform, World &world, HackerData &hacker_data, float dt, glm::vec3 direction) {
	Ray ray;
	ray.origin = transform.get_global_position() + glm::vec3(0.0f, 0.0f, -1.0f);
	ray.direction = direction;
	ray.layer_name = "hacker";
	ray.ignore_list.emplace_back(current_camera_entity);
	glm::vec3 end = ray.origin + direction * 100.0f;
	// world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(ray.origin, end, glm::vec3(1.0f, 0.0f, 0.0f));
	HitInfo info;
	bool hit = CollisionSystem::ray_cast_layer(world, ray, info);

	if (!hit) {
		SPDLOG_ERROR("NO HIT");
		return false;
	}

	Entity hit_entity = info.entity;

	if (info.entity == 0) {
		SPDLOG_ERROR("Hacker raycast hit entity 0");
		return false;
	}

	if (!world.has_component<Interactable>(hit_entity)) {
		SPDLOG_ERROR("Hacker raycast hit entity {} without interactable component", hit_entity);
		return false;
	}

	auto &interactable = world.get_component<Interactable>(hit_entity);

	if (!(interactable.type == InteractionType::Hacker)) {
		SPDLOG_WARN("Entity {} is not a hacker interactable", hit_entity);
		return false;
	}

	if (!interactable.can_interact) {
		SPDLOG_WARN("Entity {} cannot be interacted with", hit_entity);
		return false;
	}

	if (interactable.interaction == Interaction::HackerCameraJump) {
		jump_to_camera(world, hacker_data, hit_entity);
	}

	interactable.triggered = true;
	return true;
}

bool HackerSystem::jump_to_camera(World &world, HackerData &hacker_data, Entity camera_entity) {
	auto &camera_tf = world.get_component<Transform>(hacker_data.camera);
	auto &new_camera_tf = world.get_component<Transform>(camera_entity);

	camera_tf.set_position(new_camera_tf.get_global_position());
	camera_tf.set_orientation(new_camera_tf.get_global_orientation());

	is_on_camera = true;
	current_camera_entity = camera_entity;

	return true;
}

void HackerSystem::go_back_to_scorpion(World &world, HackerData &hacker_data) {
	current_camera_entity = 0;
	is_on_camera = false;
}

void HackerSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<HackerData>());

	world.set_system_component_whitelist<HackerSystem>(whitelist);

	previous_velocity = { 0, 0, 0 };

	ui_name = "hacker_ui";

	auto &rm = ResourceManager::get();
	auto crosshair_tex = rm.load_texture(asset_path("crosshair.ktx2").c_str());

	auto &ui = UIManager::get();
	ui.create_ui_scene(ui_name);
	ui.activate_ui_scene(ui_name);

	// anchor at the center of hacker's half of screen
	auto &root_anchor = ui.add_ui_anchor(ui_name, "root_anchor");
	root_anchor.is_screen_space = true;
	root_anchor.x = 0.75f;
	root_anchor.y = 0.5f;
	root_anchor.display = true;
	ui.add_as_root(ui_name, "root_anchor");

	auto &crosshair = ui.add_ui_image(ui_name, "crosshair");
	crosshair.texture = crosshair_tex;
	crosshair.size = glm::vec2(50.0f);

	ui.add_to_root(ui_name, "crosshair", "root_anchor");
}

void HackerSystem::update(World &world, float dt) {
	ZoneScopedN("HackerSystem::update");
	InputManager &input_manager = InputManager::get();
	;
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();
	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &hacker_data = world.get_component<HackerData>(entity);
		auto &camera_pivot_tf = world.get_component<Transform>(hacker_data.camera_pivot);
		auto &model_tf = world.get_component<Transform>(hacker_data.model);
		auto &scorpion_camera_tf = world.get_component<Transform>(hacker_data.scorpion_camera_transform);
		auto &camera_tf = world.get_component<Transform>(hacker_data.camera);
		auto &animation_instance = world.get_component<AnimationInstance>(hacker_data.model);

		if (!is_on_camera) {
			camera_tf.set_position(scorpion_camera_tf.get_global_position());
			camera_tf.set_orientation(scorpion_camera_tf.get_global_orientation());
		}

		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		glm::vec3 camera_forward;
		if (!is_on_camera) {
			camera_forward = camera_pivot_tf.get_global_forward();
		} else {
			camera_forward = camera_tf.get_global_forward();
		}
		glm::vec3 real_camera_forward = camera_forward;

		camera_forward.y = 0.0f;
		camera_forward = glm::normalize(camera_forward);
		auto camera_right = camera_pivot_tf.get_global_right();

		if (!is_on_camera) {
			world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(
					camera_tf.get_global_position() + glm::vec3(0.0f, 0.0f, -1.0f),
					camera_tf.get_global_position() + glm::vec3(0.0f, 0.0f, -1.0f) + real_camera_forward * 100.0f,
					glm::vec3(1.0f, 0.0f, 0.0f));
		} else {
			world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(
					camera_tf.get_global_position() + glm::vec3(0.0f, 0.0f, -1.0f),
					camera_tf.get_global_position() + glm::vec3(0.0f, 0.0f, 1.0f) - real_camera_forward * 100.0f,
					glm::vec3(1.0f, 0.0f, 0.0f));
		}

		glm::vec3 acc_direction = { 0, 0, 0 };
		if (!is_on_camera) {
			if (input_manager.is_action_pressed("hacker_move_forward")) {
				acc_direction += camera_forward;
			}
			if (input_manager.is_action_pressed("hacker_move_backward")) {
				acc_direction -= camera_forward;
			}
			if (input_manager.is_action_pressed("hacker_move_left")) {
				acc_direction += camera_right;
			}
			if (input_manager.is_action_pressed("hacker_move_right")) {
				acc_direction -= camera_right;
			}
		}

		if (*CVarSystem::get()->get_int_cvar("debug_camera.use")) {
			acc_direction = glm::vec3(0.0f);
		}

		if (input_manager.is_action_just_pressed("mouse_left")) {
			if (!is_on_camera) {
				shoot_raycast(camera_tf, world, hacker_data, dt, real_camera_forward);
			} else {
				shoot_raycast(camera_tf, world, hacker_data, dt, -real_camera_forward);
			}
		}
		if (input_manager.is_action_just_pressed("mouse_right")) {
			go_back_to_scorpion(world, hacker_data);
		}
		glm::normalize(acc_direction);
		glm::vec3 velocity = move_ground(acc_direction, previous_velocity, dt);
		if (glm::dot(velocity, velocity) > physics_manager.get_epsilon()) {
			if (animation_instance.animation_handle.id !=
					resource_manager.get_animation_handle("scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_walk.anim")
							.id) {
				animation_manager.change_animation(
						hacker_data.model, "scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_walk.anim");
			}
		} else {
			if (animation_instance.animation_handle.id !=
					resource_manager.get_animation_handle("scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_idle.anim")
							.id) {
				animation_manager.change_animation(
						hacker_data.model, "scorpion/scorpion_idle_ANIM_GLTF/scorpion_idle_00_idle.anim");
			}
		}

		transform.add_position(glm::vec3(velocity.x, 0.0, velocity.z));

		glm::vec2 mouse_delta = glm::vec2(0.0f);
		mouse_delta.x = input_manager.get_axis("hacker_look_left", "hacker_look_right");
		mouse_delta.y = input_manager.get_axis("hacker_look_up", "hacker_look_down");
		mouse_delta *= 20.0;

		if (!*CVarSystem::get()->get_int_cvar("game.controlling_agent")) {
			if (glm::length2(mouse_delta) < glm::length2(input_manager.get_mouse_delta())) {
				mouse_delta = input_manager.get_mouse_delta();
			}
		}

		if (*CVarSystem::get()->get_int_cvar("debug_camera.use")) {
			mouse_delta = glm::vec2(0.0f);
		}

		if (!is_on_camera) {
			camera_pivot_tf.add_euler_rot(
					glm::vec3(mouse_delta.y, 0.0f, 0.0f) * cvar_hacker_camera_sensitivity.get() * dt);
			camera_pivot_tf.add_global_euler_rot(
					glm::vec3(0.0f, -mouse_delta.x, 0.0f) * cvar_hacker_camera_sensitivity.get() * dt);
		} else {
			camera_tf.add_euler_rot(glm::vec3(-mouse_delta.y, 0.0f, 0.0f) * cvar_hacker_camera_sensitivity.get() * dt);
			camera_tf.add_global_euler_rot(
					glm::vec3(0.0f, -mouse_delta.x, 0.0f) * cvar_hacker_camera_sensitivity.get() * dt);
		}

		static glm::vec3 last_position = transform.position;

		// Lerp model_tf towards movement direction
		glm::vec3 delta_position = transform.position - last_position;
		delta_position.y = 0.0f;
		if (glm::length2(delta_position) > 0.0001f) {
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
		// world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(ray.origin, end);
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