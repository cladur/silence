#include "agent_system.h"
#include "components/agent_data_component.h"
#include "components/collider_capsule.h"
#include "components/enemy_data_component.h"
#include "components/platform_component.h"
#include "components/transform_component.h"
#include "cvars/cvars.h"
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

AutoCVarFloat cvar_agent_interaction_range("agent.interaction_range", "range of interaction", 1.0f);

AutoCVarFloat cvar_agent_attack_range("agent.attack_range", "range of attack", 1.5f);

AutoCVarFloat cvar_agent_attack_angle("agent.attack_angle", "angle of attack", 70.0f);

AutoCVarFloat cvar_agent_lock_time("agent.lock_time", "minimal time of animation in ms", 500.0f);

AutoCVarFloat cvar_agent_camera_back("agent.cam_back", "distance of camera from player", -1.5f);

AutoCVarFloat cvar_agent_animation_speed("agent.animation_speed", "speed of animation", 32.0f);

AutoCVarFloat cvar_camera_sensitivity("settings.camera_sensitivity", "camera sensitivity", 0.1f);

void AgentSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<AgentData>());

	world.set_system_component_whitelist<AgentSystem>(whitelist);

	ui_name = "agent_ui";

	auto &rm = ResourceManager::get();
	auto dot_tex = rm.load_texture(asset_path("dot.ktx2").c_str());

	auto &ui = UIManager::get();
	ui.create_ui_scene(ui_name);
	ui.activate_ui_scene(ui_name);

	// anchor at the center of hacker's half of screen
	auto &root_anchor = ui.add_ui_anchor(ui_name, "root_anchor");
	root_anchor.is_screen_space = true;
	root_anchor.x = 0.25;
	root_anchor.y = 0.5f;
	root_anchor.display = true;
	ui.add_as_root(ui_name, "root_anchor");

	auto &dot = ui.add_ui_image(ui_name, "dot");
	dot.texture = dot_tex;
	dot.size = glm::vec2(2.0f);
	dot.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.6f);

	ui.add_to_root(ui_name, "dot", "root_anchor");

	ui_interaction_text = &ui.add_ui_text(ui_name, "interaction_text");
	ui_interaction_text->text = "";
	ui_interaction_text->is_screen_space = true;
	ui_interaction_text->size = glm::vec2(0.5f);
	ui_interaction_text->position = glm::vec3(150.0f, 3.0f, 0.0f);
	ui_interaction_text->centered_y = true;
	ui.add_to_root(ui_name, "interaction_text", "root_anchor");

	ui_kill_text = &ui.add_ui_text(ui_name, "kill_text");
	ui_kill_text->text = "";
	ui_kill_text->is_screen_space = true;
	ui_kill_text->size = glm::vec2(0.5f);
	ui_kill_text->position = glm::vec3(-150.0f, 3.0f, 0.0f);
	ui_kill_text->centered_y = true;
	ui.add_to_root(ui_name, "kill_text", "root_anchor");
}

void AgentSystem::update(World &world, float dt) {
	ZoneScopedN("AgentSystem::update");
	InputManager &input_manager = InputManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();
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
		auto &is_climbing = agent_data.is_climbing;

		static glm::vec3 last_position = transform.position;
		glm::vec3 velocity = transform.position - last_position;
		velocity.y = 0.0f;
		const float speed = glm::length(velocity);

		if (first_frame) {
			default_fov = camera.fov;
			camera_pivot_tf.set_orientation(glm::quat(1, 0, 0, 0));
			current_rotation_y_camera_pivot = 0;
			first_frame = false;
		}

		if (world.has_component<Highlight>(agent_data.model)) {
			auto &highlight = world.get_component<Highlight>(agent_data.model);
			highlight.highlighted = true;
		}

		//TODO: replace hard coded values with one derived from collider
		if (input_manager.is_action_just_pressed("agent_crouch")) {
			if (!is_crouching) {
				is_crouching = true;
				GameplayManager::get().set_agent_crouch(is_crouching);
				capsule_collider.end.y = 0.65f;
			} else {
				Ray ray{};
				ray.origin = transform.get_global_position() + glm::vec3(0.0f, 1.0f, 0.0f);
				ray.direction = model_tf.get_global_up();
				ray.ignore_list.emplace_back(entity);
				ray.layer_name = "agent";
				glm::vec3 end = ray.origin + ray.direction;
				HitInfo info;
				bool hit = CollisionSystem::ray_cast_layer(world, ray, info);
				if (!hit || info.distance > 0.8f) {
					capsule_collider.end.y = 1.3f;
					is_crouching = false;
					GameplayManager::get().set_agent_crouch(is_crouching);
				}
			}
		}

		if (!is_climbing) {
			animation_instance.blend_time_ms = 700.0f;
		}

		if (speed > 0.02f && !agent_data.locked_movement) {
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
			//TODO: works for current speed values, but should be replaced with animation accurate value
			animation_instance.ticks_per_second = 500.f + (1000.f * speed * cvar_agent_animation_speed.get());
		} else if (animation_timer >=
				resource_manager.get_animation(animation_instance.animation_handle).get_duration()) {
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
			animation_instance.ticks_per_second = 1000.f;

			agent_data.locked_movement = false;
		}

		//Camera
		float def_cam_z = cvar_agent_camera_back.get();
		if (*CVarSystem::get()->get_int_cvar("game.controlling_agent") &&
				!*CVarSystem::get()->get_int_cvar("debug_camera.use")) {
			glm::vec2 mouse_delta = input_manager.get_mouse_delta();
			float rotation_y = mouse_delta.y * cvar_camera_sensitivity.get() * dt * camera_sens_modifier;
			if (current_rotation_y_camera_pivot + rotation_y > -1.5f &&
					current_rotation_y_camera_pivot + rotation_y < 1.5f) {
				current_rotation_y_camera_pivot += rotation_y;
				camera_pivot_tf.add_euler_rot(glm::vec3(rotation_y, 0.0f, 0.0f));
			}

			camera_pivot_tf.add_global_euler_rot(
					glm::vec3(0.0f, -mouse_delta.x, 0.0f) * cvar_camera_sensitivity.get() * dt * camera_sens_modifier);

			//check how far behind camera can be
			Ray ray{};
			ray.layer_name = "default";
			ray.ignore_list.emplace_back(entity);
			ray.origin = spring_arm_tf.get_global_position();
			ray.direction = -spring_arm_tf.get_global_forward();
			glm::vec3 end = ray.origin + ray.direction;
			HitInfo info;
			if (CollisionSystem::ray_cast_layer(world, ray, info)) {
				if (info.distance < -def_cam_z) {
					camera_tf.set_position({ 0.0f, 0.0f, -info.distance / 1.3f });
				} else {
					camera_tf.set_position({ 0.0f, 0.0f, def_cam_z });
				}
			} else {
				camera_tf.set_position({ 0.0f, 0.0f, def_cam_z });
			}
			//TODO: raycast should be able to check 2 layers at once
			Ray ray_floor{};
			ray_floor.layer_name = "camera";
			ray_floor.origin = spring_arm_tf.get_global_position();
			ray_floor.direction = -spring_arm_tf.get_global_forward();
			end = ray.origin + ray.direction;
			if (CollisionSystem::ray_cast_layer(world, ray_floor, info)) {
				if (info.distance < -def_cam_z) {
					camera_tf.set_position({ 0.0f, 0.0f, -info.distance });
				}
			}
		}

		ui_interaction_text->text = "";
		ui_kill_text->text = "";

		if (!is_zooming) {
			//Agent Climbing
			if (input_manager.is_action_just_pressed("agent_climb")) {
				Ray ray{};
				ray.origin = transform.get_global_position() + glm::vec3(0.0f, 1.4f, 0.0f) + model_tf.get_forward();
				ray.ignore_list.emplace_back(entity);
				ray.layer_name = "default";
				ray.direction = -transform.get_up();
				glm::vec3 end = ray.origin + ray.direction;
				HitInfo info;
				dd.draw_arrow(ray.origin, end, { 1.0f, 0.0f, 0.0f });
				if (CollisionSystem::ray_cast_layer(world, ray, info)) {
					float obstacle_height = 1.4f - info.distance;
					SPDLOG_INFO(obstacle_height);
					SPDLOG_INFO(info.entity);
					if (obstacle_height > 0.6f && obstacle_height < 0.7f) {
						auto animation_handle =
								resource_manager.get_animation_handle("agent/agent_ANIM_GLTF/agent_jump_up.anim");
						if (animation_instance.animation_handle.id != animation_handle.id) {
							animation_instance.ticks_per_second = 1000.f;
							animation_timer = 0;
							agent_data.locked_movement = true;
							is_climbing = true;
							animation_manager.change_animation(
									agent_data.model, "agent/agent_ANIM_GLTF/agent_jump_up.anim");
						}
					}
				}
			}
			if (!is_climbing) {
				//Agent interaction
				bool interaction_triggered = input_manager.is_action_just_pressed("agent_interact");
				ColliderSphere sphere{};
				sphere.radius = cvar_agent_interaction_range.get();
				sphere.center = model_tf.get_global_position() + glm::vec3{ 0.0f, 1.0f, 0.0f };
				auto colliders = PhysicsManager::get().overlap_sphere(world, sphere, "agent");
				// dd.draw_sphere(sphere.center, sphere.radius);
				if (!colliders.empty()) {
					Entity closest_interactable;
					float min_distance = cvar_agent_interaction_range.get() + 1.0f;
					bool interactable_found = false;
					for (Entity found_entity : colliders) {
						if (world.has_component<Interactable>(found_entity)) {
							auto &interactable = world.get_component<Interactable>(found_entity);
							if ((interactable.type == InteractionType::Agent) && interactable.can_interact) {
								auto &transform = world.get_component<Transform>(found_entity);
								if ((glm::distance(sphere.center, transform.get_global_position())) < min_distance) {
									Ray ray{};
									ray.layer_name = "default";
									ray.ignore_list.emplace_back(entity);
									ray.origin = sphere.center;
									ray.direction = transform.get_global_position() - sphere.center;
									ray.length = 3.0f;
									HitInfo info;
									glm::vec3 end = ray.origin + ray.direction;
									// dd.draw_arrow(ray.origin, end, { 1.0f, 0.0f, 0.0f });
									CollisionSystem::ray_cast_layer(world, ray, info);
									if (info.entity == found_entity) {
										interactable_found = true;
										closest_interactable = found_entity;
										min_distance = glm::distance(sphere.center, transform.position);
									}
								}
							}
						}
					}

					if (interactable_found) {
						auto &interactable = world.get_component<Interactable>(closest_interactable);
						if (world.has_component<Highlight>(closest_interactable)) {
							auto &highlight = world.get_component<Highlight>(closest_interactable);
							highlight.highlighted = true;
						}
						ui_interaction_text->text = "Press E to interact";
						if (interaction_triggered) {
							interactable.triggered = true;
							auto animation_handle = resource_manager.get_animation_handle(
									"agent/agent_ANIM_GLTF/agent_interaction.anim");
							if (animation_instance.animation_handle.id != animation_handle.id) {
								auto &transform = world.get_component<Transform>(closest_interactable);
								auto model_position = model_tf.get_global_position();
								glm::vec3 direction = model_position - transform.get_global_position();
								direction.y = 0.0f;
								direction = glm::normalize(direction);
								//auto rotation_matrix = glm::mat3(view_matrix);
								//TODO: rotate model towards interactable
								//model_tf.set_orientation(glm::quat(rotation_matrix));

								animation_instance.ticks_per_second = 1000.f;
								animation_timer = 0;
								agent_data.locked_movement = true;
								animation_manager.change_animation(
										agent_data.model, "agent/agent_ANIM_GLTF/agent_interaction.anim");
							}
						}
					}
				}

				//Agent attack
				//ray.origin = transform.get_global_position() + glm::vec3(0.0f, 1.0f, 0.0f);
				//ray.ignore_list.emplace_back(entity);
				Ray ray = {};
				ray.direction = model_tf.get_forward();
				ray.origin = transform.get_global_position() + glm::vec3(0.0f, 1.0f, 0.0f);
				ray.ignore_list.emplace_back(entity);
				ray.length = 3.0f;

				glm::vec3 end = ray.origin + ray.direction;
				HitInfo info;
				world.get_parent_scene()->get_render_scene().debug_draw.draw_arrow(ray.origin, end, { 255, 0, 0 });
				if (CollisionSystem::ray_cast_layer(world, ray, info)) {
					if (info.distance < cvar_agent_attack_range.get()) {
						if (world.has_component<EnemyData>(info.entity)) {
							auto &enemy = world.get_component<EnemyData>(info.entity);
							auto &enemy_tf = world.get_component<Transform>(info.entity);
							bool behind_enemy = glm::dot(enemy_tf.get_forward(), model_tf.get_forward()) >
									glm::cos(glm::radians(cvar_agent_attack_angle.get()));
							if (behind_enemy && enemy.state_machine.get_current_state() != "dying") {
								//TODO: pull out knife or other indicator that agent can attack
								ui_kill_text->text = "[LMB] Kill";
								//auto &enemy_tf = world.get_component<Transform>(info.entity);
								if (input_manager.is_action_just_pressed("mouse_left")) {
									auto animation_handle = resource_manager.get_animation_handle(
											"agent/agent_ANIM_GLTF/agent_stab.anim");
									if (animation_instance.animation_handle.id != animation_handle.id) {
										animation_instance.ticks_per_second = 1000.f;
										animation_timer = 0;
										agent_data.locked_movement = true;
										animation_manager.change_animation(
												agent_data.model, "agent/agent_ANIM_GLTF/agent_stab.anim");
									}
									enemy.state_machine.set_state("dying");
								}
							}
						}
					}
				}
			}
		}

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

			glm::vec3 end = tag_ray.origin + tag_ray.direction;
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

		if (animation_timer < resource_manager.get_animation(animation_instance.animation_handle).get_duration()) {
			animation_timer += (dt * 1000);
			if (is_climbing &&
					((animation_timer + 100.f) >
							resource_manager.get_animation(animation_instance.animation_handle).get_duration())) {
				//climbing animation should end here, otherwise it will start to loop
				is_climbing = false;
				animation_timer += 100.f;

				Ray ray{};
				ray.origin = transform.get_global_position() + glm::vec3(0.0f, 1.4f, 0.0f) + model_tf.get_forward();
				ray.ignore_list.emplace_back(entity);
				ray.layer_name = "default";
				ray.direction = -transform.get_up();
				glm::vec3 end = ray.origin + ray.direction;
				HitInfo info;
				dd.draw_arrow(ray.origin, end, { 1.0f, 0.0f, 0.0f });
				if (CollisionSystem::ray_cast_layer(world, ray, info)) {
					transform.set_position(info.point + glm::vec3{ 0.0f, 0.2f, 0.0f });
				}

				animation_instance.blend_time_ms = 0.0f;
				animation_manager.change_animation(agent_data.model, "agent/agent_ANIM_GLTF/agent_idle.anim");
			}
		}

		AudioManager::get().set_3d_listener_attributes(SILENCE_FMOD_LISTENER_AGENT, camera_tf.get_global_position(),
				glm::vec3(0.0f), camera_tf.get_global_forward(), camera_tf.get_global_up());

		last_position = transform.position;
	}
}