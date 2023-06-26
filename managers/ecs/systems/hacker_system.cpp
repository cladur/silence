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
#include <ai/state_machine/states/enemy/enemy_utils.h>
#include <audio/audio_manager.h>
#include <render/transparent_elements/ui_manager.h>
#include <spdlog/spdlog.h>
#include <glm/common.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

AutoCVarFloat cvar_hacker_camera_sensitivity(
		"settings.hacker_camera_sensitivity", "camera sensitivity", 0.1f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_camera_back("hacker.cam_back", "distance of camera from player", -1.5f);

AutoCVarFloat cvar_hacker_camera_max_rotation_x(
		"hacker.hacker_camera_max_rotation_x", "camera max rotation X in degrees", 75.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_camera_max_rotation_y(
		"hacker.hacker_camera_max_rotation_y", "camera max rotation Y in degrees", 30.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_max_rotation_x(
		"hacker.hacker_max_rotation_x", "max rotation X in degrees", 25.0f, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_hacker_min_rotation_x(
		"hacker.hacker_min_rotation_x", "min rotation X in degrees", -55.0f, CVarFlags::EditCheckbox);

AutoCVarInt cvar_hacker_on_keyboard(
		"settings.hacker_on_keyboard", "Control hacker with keyboard + mouse", 0, CVarFlags::EditCheckbox);

AutoCVarFloat cvar_viewspace_offset("hacker.viewspace_offset", "offset from viewspace", 0.5f, CVarFlags::EditFloatDrag);

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

	ui_interaction_text->text = "";
	interaction_sprite->display = false;

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

	auto &interactable = world.get_component<Interactable>(hit_entity);
	auto &hit_transform = world.get_component<Transform>(hit_entity);

	glm::vec2 render_extent = world.get_parent_scene()->get_render_scene().render_extent;

	glm::vec2 screen_pos = enemy_utils::transform_to_screen(
			hit_transform.get_global_position(), world.get_parent_scene()->get_render_scene(), true);

	interaction_sprite->position.x = screen_pos.x + 96.0f;
	interaction_sprite->position.y = screen_pos.y + 0.0f;
	interaction_sprite->display = true;

	// take interactable.interaction_text and splt it by space

	std::istringstream iss(interactable.interaction_text);
	std::vector<std::string> words;
	std::string word;

	while (std::getline(iss, word, ' ')) {
		words.push_back(word);
	}

	if (words.size() != 1) {
		ui_button_hint->text = words[0];
		if (words.size() > 2) {
			for (int i = 1; i < words.size(); i++) {
				ui_interaction_text->text += " " + words[i];
			}
		} else {
			ui_interaction_text->text = words[1];
		}
	} else {
		ui_button_hint->text = "";
		ui_interaction_text->text = words[0];
	}

	float size = 0.6f - (words[0].size() / 10.0f);
	ui_button_hint->size = glm::vec2(size);

	if (interaction_sprite->position.x > render_extent.x / 2.0f - 130.f) {
		interaction_sprite->position.x = render_extent.x / 2.0f - 130.0f;
	}
	if (interaction_sprite->position.y > render_extent.y / 2.0f - 40.f) {
		interaction_sprite->position.y = render_extent.y / 2.0f - 40.0f;
	}
	if (interaction_sprite->position.x < -render_extent.x / 2.0f + 130.f) {
		interaction_sprite->position.x = -render_extent.x / 2.0f + 130.0f;
	}
	if (interaction_sprite->position.y < -render_extent.y / 2.0f + 40.f) {
		interaction_sprite->position.y = -render_extent.y / 2.0f + 40.0f;
	}

	if (world.has_component<Highlight>(hit_entity)) {
		auto &highlight = world.get_component<Highlight>(hit_entity);
		highlight.highlighted = true;

		if (!interactable.can_interact || !(interactable.type == InteractionType::Hacker)) {
			ui_button_hint->text = "";
			ui_interaction_text->text = "";
			highlight.highlighted = false;
			interaction_sprite->display = false;
		}

	} else {
		if (!interactable.can_interact || !(interactable.type == InteractionType::Hacker)) {
			ui_button_hint->text = "";
			ui_interaction_text->text = "";
			interaction_sprite->display = false;
		}
		SPDLOG_ERROR("Hacker raycast hit entity {} without highlight component", hit_entity);
	}

	if (trigger && interactable.can_interact && (interactable.type == InteractionType::Hacker)) {
		if (interactable.interaction == Interaction::HackerCameraJump) {
			jump_to_camera(world, hacker_data, hit_entity);
		}

		interactable.triggered = true;
		AudioManager::get().play_one_shot_2d(hacker_data.hack_sound);
	}

	return true;
}

bool HackerSystem::jump_to_camera(World &world, HackerData &hacker_data, Entity camera_entity) {
	auto &detection_camera = world.get_component<DetectionCamera>(camera_entity);
	auto camera_model_entity = detection_camera.camera_model;
	auto &camera_tf = world.get_component<Transform>(hacker_data.camera);
	auto &new_camera_tf = world.get_component<Transform>(camera_model_entity);

	before_jump_orientation = camera_tf.get_global_orientation();

	if (detection_camera.friendly_time_left != 0.0f) {
		return false;
	}

	detection_camera.is_active = false;
	detection_camera.detection_level = 0.0f;
	detection_camera.detection_target = DetectionTarget::NONE;

	new_camera_tf.set_orientation(detection_camera.starting_orientation);

	AudioManager::get().play_one_shot_2d(hacker_data.hack_sound);

	camera_tf.set_position(new_camera_tf.get_global_position() + -(new_camera_tf.get_forward()));
	camera_tf.set_orientation(new_camera_tf.get_global_orientation());

	is_on_camera = true;
	hacker_data.is_on_camera = true;
	current_camera_entity = camera_entity;
	current_camera_model_entity = camera_model_entity;
	starting_camera_orientation = world.get_component<Transform>(camera_model_entity).get_global_orientation();

	return true;
}

void HackerSystem::go_back_to_scorpion(World &world, HackerData &hacker_data) {
	if (!is_on_camera) {
		return;
	}

	auto &detection_camera = world.get_component<DetectionCamera>(current_camera_entity);
	auto &camera_billboard = world.get_component<Billboard>(current_camera_entity);
	auto &camera_tf = world.get_component<Transform>(hacker_data.camera);

	detection_camera.friendly_time_left = *CVarSystem::get()->get_float_cvar("enemy_camera.friendly_time");
	world.get_component<Transform>(detection_camera.camera_model)
			.set_orientation(detection_camera.starting_orientation);

	AudioManager::get().play_one_shot_2d(hacker_data.hack_sound);

	camera_tf.set_orientation(before_jump_orientation);

	current_rotation_x = 0.0f;
	current_rotation_y = 0.0f;

	current_camera_entity = 0;
	current_camera_model_entity = 0;
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

	interaction_sprite = &ui.add_ui_image(ui_name, "interaction_sprite");
	interaction_sprite->texture = rm.load_texture(asset_path("interaction.ktx2").c_str(), false);
	interaction_sprite->size = glm::vec2(256.0f);
	interaction_sprite->color = glm::vec4(1.0f, 1.0f, 1.0f, 0.4f);
	interaction_sprite->is_screen_space = true;
	interaction_sprite->position = glm::vec3(0.0f, 0.0f, 0.0f);
	interaction_sprite->display = false;
	ui.add_to_root(ui_name, "interaction_sprite", "root_anchor");

	ui_interaction_text = &ui.add_ui_text(ui_name, "interaction_text");
	ui_interaction_text->text = "";
	ui_interaction_text->is_screen_space = true;
	ui_interaction_text->size = glm::vec2(0.5f);
	ui_interaction_text->position = glm::vec3(10.0f, 16.0f, 0.0f);
	ui_interaction_text->color = glm::vec4(1.0f, 1.0f, 1.0f, 0.6f);
	ui_interaction_text->centered_y = true;
	ui_interaction_text->centered_x = true;

	interaction_sprite->add_child(*ui_interaction_text);

	ui_button_hint = &ui.add_ui_text(ui_name, "button_hint");
	ui_button_hint->text = "";
	ui_button_hint->is_screen_space = true;
	ui_button_hint->size = glm::vec2(0.5f);
	ui_button_hint->position = glm::vec3(-96.0f, 1.0f, 0.0f);
	ui_button_hint->color = glm::vec4(1.0f, 1.0f, 1.0f, 0.6f);
	ui_button_hint->centered_y = true;
	ui_button_hint->centered_x = true;

	interaction_sprite->add_child(*ui_button_hint);

	auto &main_anchor = ui.add_ui_anchor(ui_name, "main_anchor");
	main_anchor.is_screen_space = true;
	main_anchor.x = 0.5f;
	main_anchor.y = 0.95f;
	main_anchor.display = true;
	ui.add_as_root(ui_name, "main_anchor");

	main_text = &ui.add_ui_text(ui_name, "main_text");
	main_text->text = "";
	//main_text->is_screen_space = true;
	main_text->size = glm::vec2(1.0f);
	main_text->position = glm::vec3(0.0f, 0.0f, 0.0f);
	main_text->centered_y = true;
	ui.add_to_root(ui_name, "main_text", "main_anchor");
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
			camera_model_tf = &world.get_component<Transform>(current_camera_model_entity);
		}

		if (first_frame) {
			default_fov = camera.fov;
			first_frame = false;
			camera_pivot_tf.set_orientation(glm::quat(1, 0, 0, 0));
			//scorpion_camera_tf.set_euler_rot(glm::vec3{ 0.0f, 3.14f, 0.0f });
			starting_camera_pivot_orientation = camera_pivot_tf.get_global_orientation();
		}

		if (world.has_component<Highlight>(hacker_data.model)) {
			auto &highlight = world.get_component<Highlight>(hacker_data.model);
			highlight.highlighted = true;
		}

		if (!is_on_camera) {
			camera_tf.set_position(scorpion_camera_tf.get_global_position());
			camera_tf.set_orientation(scorpion_camera_tf.get_global_orientation());
		}

		auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;

		glm::vec3 camera_forward = -camera_tf.get_global_forward();
		glm::vec3 real_camera_forward = camera_forward;

		camera_forward.y = 0.0f;
		camera_forward = glm::normalize(camera_forward);
		auto camera_right = camera_pivot_tf.get_global_right();

		// ZOOMING LOGIC
		bool zoom_triggered = false;
		if (hacker_data.gamepad >= 0) {
			zoom_triggered = input_manager.is_action_pressed("hacker_zoom_camera", hacker_data.gamepad);
		} else {
			zoom_triggered = input_manager.is_action_pressed("hacker_zoom_camera");
		}
		if (zoom_triggered) {
			ui_interaction_text->text = "";
			interaction_sprite->display = false;
			is_zooming = true;
			camera.fov = glm::mix(camera.fov, 30.0f, dt * 3.0f);
			camera_sens_modifier = glm::mix(camera_sens_modifier, 0.3f, dt * 3.0f);
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

		bool exit_triggered = false;
		if (hacker_data.gamepad >= 0) {
			exit_triggered = input_manager.is_action_just_pressed("hacker_exit_camera", hacker_data.gamepad);
		} else {
			exit_triggered = input_manager.is_action_just_pressed("hacker_exit_camera");
		}

		if (exit_triggered) {
			go_back_to_scorpion(world, hacker_data);
		}

		auto mouse_delta = glm::vec2(0.0f);
		if (hacker_data.gamepad >= 0) {
			mouse_delta.x = input_manager.get_axis("hacker_look_left", "hacker_look_right", hacker_data.gamepad);
			mouse_delta.y = input_manager.get_axis("hacker_look_up", "hacker_look_down", hacker_data.gamepad);
		} else {
			mouse_delta.x = input_manager.get_axis("hacker_look_left", "hacker_look_right");
			mouse_delta.y = input_manager.get_axis("hacker_look_up", "hacker_look_down");
		}
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

		// CAMERA ROTATION LOGIC
		if (!is_on_camera) {
			float def_cam_z = cvar_hacker_camera_back.get();

			float rotation_y = mouse_delta.y * cvar_hacker_camera_sensitivity.get() * dt * camera_sens_modifier;
			if (current_rotation_x_camera_pivot + rotation_y > -1.4f &&
					current_rotation_x_camera_pivot + rotation_y < 1.4f) {
				current_rotation_x_camera_pivot += rotation_y;
				camera_pivot_tf.add_euler_rot(glm::vec3(rotation_y, 0.0f, 0.0f));
			}

			camera_pivot_tf.add_global_euler_rot(glm::vec3(0.0f, -mouse_delta.x, 0.0f) *
					cvar_hacker_camera_sensitivity.get() * dt * camera_sens_modifier);

			//check how far behind camera can be
			Ray ray{};
			ray.layer_name = "obstacle";
			ray.ignore_list.emplace_back(entity);
			ray.origin = camera_pivot_tf.get_global_position();
			ray.direction = -camera_pivot_tf.get_global_forward();
			HitInfo info;
			if (CollisionSystem::ray_cast_layer(world, ray, info)) {
				if (info.distance < -def_cam_z + 0.1) {
					scorpion_camera_tf.set_position({ 0.0f, 0.0f, -info.distance / 1.4f });
				} else {
					scorpion_camera_tf.set_position({ 0.0f, 0.0f, def_cam_z });
				}
			} else {
				scorpion_camera_tf.set_position({ 0.0f, 0.0f, def_cam_z });
			}

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
		bool triggered = false;
		if (hacker_data.gamepad >= 0) {
			triggered = input_manager.is_action_just_pressed("hacker_interact", hacker_data.gamepad);
		} else {
			triggered = input_manager.is_action_just_pressed("hacker_interact");
		}

		shoot_raycast(camera_tf, world, hacker_data, dt, triggered, real_camera_forward);
	}
}

void HackerSystem::reset() {
	current_rotation_x = 0.0f;
	current_rotation_y = 0.0f;

	starting_camera_pivot_orientation = glm::quat();
	starting_camera_orientation = glm::quat();
	before_jump_orientation = glm::quat();

	current_rotation_x_camera_pivot = 0.0f;
	current_rotation_x_camera = 0.0f;

	is_on_camera = false;
	current_camera_entity = 0;
	current_camera_model_entity = 0;

	bool first_frame = true;
	camera_sens_modifier = 1.0f;
	is_zooming = false;

	old_camera_pivot_tf = Transform();
}