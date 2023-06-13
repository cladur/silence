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
#include <audio/audio_manager.h>
#include <render/transparent_elements/ui_manager.h>
#include <spdlog/spdlog.h>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

AutoCVarFloat cvar_hacker_camera_sensitivity(
		"settings.hacker_camera_sensitivity", "camera sensitivity", 0.1f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_camera_max_rotation_x(
		"hacker.hacker_camera_max_rotation_x", "camera max rotation X in degrees", 75.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_camera_max_rotation_y(
		"hacker.hacker_camera_max_rotation_y", "camera max rotation Y in degrees", 30.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_max_rotation_x(
		"hacker.hacker_max_rotation_x", "max rotation X in degrees", 70.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_min_rotation_x(
		"hacker.hacker_min_rotation_x", "min rotation X in degrees", -25.0f, CVarFlags::EditCheckbox);

AutoCVarInt cvar_hacker_on_keyboard(
		"settings.hacker_on_keyboard", "Control hacker with keyboard + mouse", 0, CVarFlags::EditCheckbox);

bool HackerSystem::shoot_raycast(
		Transform &transform, World &world, HackerData &hacker_data, float dt, bool trigger, glm::vec3 direction) {
	Ray ray;
	ray.origin = transform.get_global_position();
	ray.direction = direction;
	ray.layer_name = "hacker";
	ray.ignore_list.emplace_back(current_camera_entity);
	glm::vec3 end = ray.origin + direction * 10.0f;
	HitInfo info;
	bool hit = CollisionSystem::ray_cast_layer(world, ray, info);

	ui_text->text = "";

	if (!hit) {
		//SPDLOG_ERROR("NO HIT");
		return false;
	}

	Entity hit_entity = info.entity;

	if (info.entity == 0) {
		SPDLOG_ERROR("Hacker raycast hit entity 0");
		return false;
	}

	if (!world.has_component<Interactable>(hit_entity)) {
		// SPDLOG_ERROR("Hacker raycast hit entity {} without interactable component", hit_entity);
		return false;
	}

	if (world.has_component<Highlight>(hit_entity)) {
		auto &highlight = world.get_component<Highlight>(hit_entity);
		highlight.highlighted = true;
	} else {
		SPDLOG_ERROR("Hacker raycast hit entity {} without highlight component", hit_entity);
	}

	ui_text->text = "Press LMB to interact";

	if (trigger) {
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
	}

	return true;
}

bool HackerSystem::jump_to_camera(World &world, HackerData &hacker_data, Entity camera_entity) {
	auto &detection_camera = world.get_component<DetectionCamera>(camera_entity);
	auto &camera_billboard = world.get_component<Billboard>(camera_entity);
	auto &camera_tf = world.get_component<Transform>(hacker_data.camera);
	auto &new_camera_tf = world.get_component<Transform>(camera_entity);

	before_jump_orientation = camera_tf.get_global_orientation();

	detection_camera.is_active = false;
	detection_camera.detection_level = 0.0f;
	detection_camera.detection_target = DetectionTarget::NONE;
	camera_billboard.color = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

	new_camera_tf.set_orientation(detection_camera.starting_orientation);

	camera_tf.set_position(new_camera_tf.get_global_position() + -(new_camera_tf.get_forward()));
	camera_tf.set_orientation(new_camera_tf.get_global_orientation());

	is_on_camera = true;
	hacker_data.is_on_camera = true;
	current_camera_entity = camera_entity;
	starting_camera_orientation = world.get_component<Transform>(camera_entity).get_global_orientation();

	return true;
}

void HackerSystem::go_back_to_scorpion(World &world, HackerData &hacker_data) {
	if (!is_on_camera) {
		return;
	}

	auto &detection_camera = world.get_component<DetectionCamera>(current_camera_entity);
	auto &camera_billboard = world.get_component<Billboard>(current_camera_entity);
	auto &camera_tf = world.get_component<Transform>(hacker_data.camera);

	detection_camera.is_active = true;
	camera_billboard.color = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);

	world.get_component<Transform>(current_camera_entity).set_orientation(starting_camera_orientation);
	//starting_camera_orientation = glm::quat(1, 0, 0, 0);
	//current_rotation_x = 0.0f;
	//current_rotation_y = 0.0f;
	camera_tf.set_orientation(before_jump_orientation);
	current_camera_entity = 0;
	is_on_camera = false;
	hacker_data.is_on_camera = false;
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

	ui_text = &ui.add_ui_text(ui_name, "text");
	ui_text->text = "";
	ui_text->is_screen_space = true;
	ui_text->size = glm::vec2(0.5f);
	ui_text->position = glm::vec3(150.0f, 3.0f, 0.0f);
	ui_text->centered_y = true;
	ui.add_to_root(ui_name, "text", "root_anchor");
}

void HackerSystem::update(World &world, float dt) {
	ZoneScopedN("HackerSystem::update");
	InputManager &input_manager = InputManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();
	for (const Entity entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &hacker_data = world.get_component<HackerData>(entity);
		auto &camera_pivot_tf = world.get_component<Transform>(hacker_data.camera_pivot);
		auto &scorpion_camera_tf = world.get_component<Transform>(hacker_data.scorpion_camera_transform);
		auto &camera_tf = world.get_component<Transform>(hacker_data.camera);
		auto &animation_instance = world.get_component<AnimationInstance>(hacker_data.model);
		auto &camera = world.get_component<Camera>(hacker_data.camera);

		Transform *camera_model_tf = nullptr;

		if (is_on_camera) {
			camera_model_tf = &world.get_component<Transform>(current_camera_entity);
		}

		if (first_frame) {
			default_fov = camera.fov;
			first_frame = false;
			starting_camera_pivot_orientation = camera_pivot_tf.get_global_orientation();
		}

		if (world.has_component<Highlight>(hacker_data.model)) {
			auto &highlight = world.get_component<Highlight>(hacker_data.model);
			highlight.highlighted = true;
		}

		if (!is_on_camera) {
			camera_tf.set_position(scorpion_camera_tf.get_global_position());
		}

		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		glm::vec3 camera_forward = -camera_tf.get_global_forward();
		glm::vec3 real_camera_forward = camera_forward;

		dd.draw_line(camera_tf.get_global_position(), camera_tf.get_global_position() + real_camera_forward * 100.0f);

		camera_forward.y = 0.0f;
		camera_forward = glm::normalize(camera_forward);
		auto camera_right = camera_pivot_tf.get_global_right();

		// ZOOMING LOGIC
		if (input_manager.is_action_pressed("control_camera")) {
			is_zooming = true;
			camera.fov = glm::mix(camera.fov, 30.0f, dt * 3.0f);
			camera_sens_modifier = glm::mix(camera_sens_modifier, 0.3f, dt * 3.0f);
			bool tag_trigger = input_manager.is_action_just_pressed("mouse_left");
			Ray tag_ray = {};
			tag_ray.origin = camera_tf.get_global_position();
			tag_ray.direction = -camera_tf.get_global_forward();
			tag_ray.ignore_list.emplace_back(entity);
			tag_ray.layer_name = "hacker";
			//			tag_ray.ignore_layers.emplace_back("camera");

			HitInfo info = {};
			if (CollisionSystem::ray_cast_layer(world, tag_ray, info)) {
				auto &name = world.get_component<Name>(info.entity);
				if (world.has_component<Taggable>(info.entity)) {
					auto &taggable = world.get_component<Taggable>(info.entity);

					taggable.tagging = true;
				}
			}

		} else {
			camera.fov = glm::mix(camera.fov, default_fov, dt * 7.0f);
			camera_sens_modifier = glm::mix(camera_sens_modifier, 1.0f, dt * 7.0f);
			if (glm::distance(camera.fov, default_fov) < 0.05f) {
				camera.fov = default_fov;
				camera_sens_modifier = 1.0f;
				is_zooming = false;
			}
		}

		if (input_manager.is_action_just_pressed("back_from_camera")) {
			go_back_to_scorpion(world, hacker_data);
		}

		auto mouse_delta = glm::vec2(0.0f);
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

		mouse_delta *= camera_sens_modifier;
		float camera_sensitivity = cvar_hacker_camera_sensitivity.get();
		if (!is_on_camera) {
			current_rotation_x_camera_pivot += mouse_delta.y * camera_sensitivity * dt;
			float max_rotation_x = cvar_hacker_max_rotation_x.get() * 0.017f;
			float min_rotation_x = cvar_hacker_min_rotation_x.get() * 0.017f;

			if (current_rotation_x_camera_pivot < max_rotation_x && current_rotation_x_camera_pivot > min_rotation_x) {
				camera_pivot_tf.add_euler_rot(glm::vec3(mouse_delta.y, 0.0f, 0.0f) * camera_sensitivity * dt);
				camera_tf.set_orientation(scorpion_camera_tf.get_global_orientation());
			} else {
				camera_tf.add_euler_rot(glm::vec3(-mouse_delta.y, 0.0f, 0.0f) * camera_sensitivity * dt);
				camera_tf.add_global_euler_rot(glm::vec3(0.0f, -mouse_delta.x, 0.0f) * camera_sensitivity * dt);
			}

			camera_pivot_tf.add_global_euler_rot(glm::vec3(0.0f, -mouse_delta.x, 0.0f) * camera_sensitivity * dt);
		} else {
			float new_rotation_x = current_rotation_x + (-mouse_delta.x * camera_sensitivity * dt);
			float new_rotation_y = current_rotation_y + (-mouse_delta.y * camera_sensitivity * dt);

			float max_rotation_x = cvar_hacker_camera_max_rotation_x.get() * 0.017f;
			float max_rotation_y = cvar_hacker_camera_max_rotation_y.get() * 0.017f;

			if (abs(new_rotation_x) <= max_rotation_x) {
				camera_tf.add_global_euler_rot(glm::vec3(0.0f, -mouse_delta.x, 0.0f) * camera_sensitivity * dt);
				camera_model_tf->add_global_euler_rot(glm::vec3(0.0f, -mouse_delta.x, 0.0f) * camera_sensitivity * dt);

				current_rotation_x = new_rotation_x;
			}

			if (abs(new_rotation_y) <= max_rotation_y) {
				camera_tf.add_euler_rot(glm::vec3(-mouse_delta.y, 0.0f, 0.0f) * camera_sensitivity * dt);
				camera_model_tf->add_euler_rot(glm::vec3(-mouse_delta.y, 0.0f, 0.0f) * camera_sensitivity * dt);

				current_rotation_y = new_rotation_y;
			}
		}

		// for the UI sake we need to shoot the raycast every time to know if we're even hovering over anything.
		bool triggered = input_manager.is_action_just_pressed("mouse_left");
		if (!is_zooming) {
			shoot_raycast(camera_tf, world, hacker_data, dt, triggered, real_camera_forward);
		}
	}
}