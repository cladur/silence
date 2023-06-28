#include "interactable_system.h"
#include "components/enemy_data_component.h"
#include "components/exploding_box_component.h"
#include "components/interactable_component.h"
#include "components/transform_component.h"
#include "cvars/cvars.h"
#include "ecs/systems/interactable_system.h"
#include "ecs/world.h"
#include "enemy_system.h"
#include "engine/scene.h"
#include "fmod_errors.h"
#include "physics/ecs/physics_system.h"
#include "physics/physics_manager.h"
#include <ai/state_machine/states/enemy/enemy_utils.h>
#include <audio/audio_manager.h>
#include <gameplay/gameplay_manager.h>
#include <render/transparent_elements/ui_manager.h>
#include <spdlog/spdlog.h>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>

AutoCVarFloat cvar_temporal_switch_duration(
		"interactable.temporal_switch_duration", "duration of temporal switch", 5.0f, CVarFlags::EditCheckbox);

#define FMOD_CHECK(x)                                                                                                  \
	do {                                                                                                               \
		FMOD_RESULT result = x;                                                                                        \
		if (result != FMOD_OK) {                                                                                       \
			SPDLOG_ERROR("Audio Manager: FMOD error! {} {}", result, FMOD_ErrorString(result));                        \
			abort();                                                                                                   \
		}                                                                                                              \
	} while (false)

void draw_explosion_radius(World &world, glm::vec3 centre, float radius, glm::vec3 color) {
	auto &dd = world.get_parent_scene()->get_render_scene().debug_draw;
	dd.draw_sphere(centre, radius, color);
}

static const std::string ui_name = "ui_interaction_scene";

void InteractableSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Interactable>());
	world.set_system_component_whitelist<InteractableSystem>(signature);

	explostion_event = EventReference("SFX/Explosions/electric_box");
	electric_interaction_event = EventReference("SFX/Interactions/electric_interaction");

	auto &ui = UIManager::get();
	ui.create_ui_scene(ui_name);
	ui.activate_ui_scene(ui_name);
}

void InteractableSystem::rotate_handle(World &world, float &dt, Interactable &interactable) {
	float target_angle = interactable.is_on ? -10.0f : -170.0f;
	auto &lever_transform = world.get_component<Transform>(interactable.lever);

	interactable.is_rotating = !lever_transform.lerp_rotation_towards(target_angle, glm::vec3(1.0f, 0.0f, 0.0f), dt);
}

void InteractableSystem::switch_lights(World &world, Interactable &interactable) {
	for (Entity light_entity : interactable.interaction_targets) {
		if (light_entity == 0) {
			break;
		}
		auto &light = world.get_component<Light>(light_entity);
		light.is_on = !light.is_on;
	}
}

void InteractableSystem::update(World &world, float dt) {
	auto &ui = UIManager::get();
	for (const Entity entity : entities) {
		auto &interactable = world.get_component<Interactable>(entity);
		auto &transform = world.get_component<Transform>(entity);
		Highlight *highlight = nullptr;
		if (world.has_component<Highlight>(entity)) {
			highlight = &world.get_component<Highlight>(entity);
		}
		if (interactable.interaction == Interaction::Exploding &&
				*CVarSystem::get()->get_int_cvar("debug_draw.collision.draw")) {
			auto box = world.get_component<ExplodingBox>(interactable.interaction_targets[0]);
			auto center = world.get_component<Transform>(interactable.interaction_targets[0]).get_global_position();
			draw_explosion_radius(world, center, box.explosion_radius, { 255, 0, 0 });
			draw_explosion_radius(world, center, box.distraction_radius, { 255, 128, 0 });
		}

		if (interactable.first_frame) {
			interactable.first_frame = false;

			if (interactable.interaction == Interaction::Exploding) {
				if (world.has_component<FMODEmitter>(entity)) {
					auto &sound = world.get_component<FMODEmitter>(entity);
					FMOD_CHECK(sound.event_instance->setParameterByName("buzz_volume", 0.1f));
					FMOD_CHECK(sound.event_instance->setParameterByName("electricity_volume", 1.0f));
				}
			}

			if (interactable.lever != 0) {
				auto &lever_transform = world.get_component<Transform>(interactable.lever);
				if (!interactable.is_on) {
					lever_transform.add_euler_rot(glm::vec3(glm::radians(-170.0f), 0.0f, 0.0f));
				} else {
					lever_transform.add_euler_rot(glm::vec3(glm::radians(-10.0f), 0.0f, 0.0f));
				}
			}

			if (interactable.cable_parent != 0) {
				auto &cable = world.get_component<CableParent>(interactable.cable_parent);
				cable.state = interactable.is_on ? CableState::ON : CableState::OFF;
			}

			auto &interaction_root = ui.add_ui_anchor(ui_name, std::to_string(entity) + "_ui_interaction_root");
			interaction_root.display = true;
			interaction_root.is_screen_space = true;
			interaction_root.is_billboard = false;
			if (interactable.type == InteractionType::Hacker) {
				interaction_root.x = 0.75f;
				interaction_root.y = 0.5f;
			} else {
				interaction_root.x = 0.25f;
				interaction_root.y = 0.5f;
			}
			ui.add_as_root(ui_name, std::to_string(entity) + "_ui_interaction_root");

			auto &interaction_sprite = ui.add_ui_image(ui_name, std::to_string(entity) + "_ui_interaction_sprite");
			interaction_sprite.display = false;
			interaction_sprite.is_billboard = false;
			interaction_sprite.is_screen_space = true;
			interaction_sprite.position = glm::vec3(0.0f);
			interaction_sprite.size = glm::vec2(256.0f);
			interaction_sprite.texture =
					ResourceManager::get().load_texture(asset_path("interaction_highlight.ktx2").c_str());
			interaction_sprite.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.4f);
			ui.add_to_root(ui_name, std::to_string(entity) + "_ui_interaction_sprite",
					std::to_string(entity) + "_ui_interaction_root");
		}

		if (interactable.can_interact) {
			glm::vec2 render_extent = world.get_parent_scene()->get_render_scene().render_extent;
			glm::vec3 player_cam_pos;
			auto &interaction_sprite = ui.get_ui_image(ui_name, std::to_string(entity) + "_ui_interaction_sprite");
			if (interactable.type == InteractionType::Agent) {
				uint32_t camera_entity = GameplayManager::get().get_agent_camera(world.get_parent_scene());
				player_cam_pos = world.get_component<Transform>(camera_entity).get_global_position();
				Ray ray;
				ray.origin = player_cam_pos;
				ray.direction = glm::normalize(transform.get_global_position() - player_cam_pos);
				ray.layer_name = "agent";
				ray.length = *CVarSystem::get()->get_float_cvar("agent.interaction_range") * 15.0f;
				ray.ignore_list.emplace_back(camera_entity);
				HitInfo info;
				if (CollisionSystem::ray_cast_layer(world, ray, info) &&
						world.has_component<Interactable>(info.entity)) {
					auto cam_forward = world.get_component<Transform>(camera_entity).get_global_forward();
					auto cam_forward_xz_proj = glm::normalize(glm::vec3(cam_forward.x, 0.0f, cam_forward.z));

					auto direction_xz = glm::normalize(glm::vec3(ray.direction.x, 0.0f, ray.direction.z));
					auto angle = glm::degrees(glm::acos(glm::dot(direction_xz, cam_forward_xz_proj)));

					glm::vec2 screen_pos = enemy_utils::transform_to_screen(
							transform.get_global_position(), world.get_parent_scene()->get_render_scene(), false);
					screen_pos.x += 96.0f;
					interaction_sprite.position = glm::vec3(screen_pos, 0.0f);
					interaction_sprite.display = true;

					if (interaction_sprite.position.x > render_extent.x / 2.0f + 64.0f) {
						interaction_sprite.display = false;
					} else if (interaction_sprite.position.x < -render_extent.x / 2.0f + 130.f) {
						interaction_sprite.display = false;
					}
					if (interaction_sprite.position.y > render_extent.y / 2.0f - 40.f) {
						interaction_sprite.display = false;
					} else if (interaction_sprite.position.y < -render_extent.y / 2.0f + 40.f) {
						interaction_sprite.display = false;
					}

					if (angle < 90.0f) {
						interaction_sprite.display = false;
					}
				} else {
					interaction_sprite.display = false;
				}
			} else { // type == HACKER
				uint32_t camera_entity = GameplayManager::get().get_hacker_camera(world.get_parent_scene());
				player_cam_pos = world.get_component<Transform>(camera_entity).get_global_position();
				Ray ray;
				ray.origin = player_cam_pos;
				ray.direction = glm::normalize(transform.get_global_position() - player_cam_pos);
				ray.layer_name = "hacker";
				ray.length = *CVarSystem::get()->get_float_cvar("hacker.range");
				ray.ignore_list.emplace_back(camera_entity);
				HitInfo info;

				if (CollisionSystem::ray_cast_layer(world, ray, info) &&
						world.has_component<Interactable>(info.entity)) {
					auto cam_forward = world.get_component<Transform>(camera_entity).get_global_forward();
					auto cam_forward_xz_proj = glm::normalize(glm::vec3(cam_forward.x, 0.0f, cam_forward.z));

					auto direction_xz = glm::normalize(glm::vec3(ray.direction.x, 0.0f, ray.direction.z));
					auto angle = glm::degrees(glm::acos(glm::dot(direction_xz, cam_forward_xz_proj)));

					glm::vec2 screen_pos = enemy_utils::transform_to_screen(
							transform.get_global_position(), world.get_parent_scene()->get_render_scene(), true);
					screen_pos.x += 96.0f;
					interaction_sprite.position = glm::vec3(screen_pos, 0.0f);
					interaction_sprite.display = true;

					if (interaction_sprite.position.x > render_extent.x / 2.0f + 130.f) {
						interaction_sprite.display = false;
					} else if (interaction_sprite.position.x < -render_extent.x / 2.0f + 130.f) {
						interaction_sprite.display = false;
					}

					if (interaction_sprite.position.y > render_extent.y / 2.0f - 40.f) {
						interaction_sprite.display = false;
					} else if (interaction_sprite.position.y < -render_extent.y / 2.0f + 40.f) {
						interaction_sprite.display = false;
					}

					if (angle < 90.0f) {
						interaction_sprite.display = false;
					}
				} else {
					interaction_sprite.display = false;
				}
			}

			if (highlight != nullptr) {
				if (highlight->highlighted) {
					interaction_sprite.display = false;
				}
			}
		}

		if (interactable.is_rotating) {
			rotate_handle(world, dt, interactable);
		}

		if (interactable.is_powering_up) {
			interactable.temporal_switch_time += dt;

			if (interactable.cable_parent != 0) {
				auto &cable = world.get_component<CableParent>(interactable.cable_parent);
				auto current_switch_time = interactable.temporal_switch_time;

				if (current_switch_time != 0.0f) {
					cable.color_value = current_switch_time / cvar_temporal_switch_duration.get();
				}
			}

			if (interactable.temporal_switch_time >= cvar_temporal_switch_duration.get()) {
				interactable.temporal_switch_time = 0.0f;
				interactable.can_interact = true;
				interactable.is_on = true;
				interactable.is_rotating = true;
				interactable.is_powering_up = false;

				switch_lights(world, interactable);

				if (interactable.enemy_entity != 0) {
					auto &enemy = world.get_component<EnemyData>(interactable.enemy_entity);
					enemy.is_blinded = false;
					break;
				}

				if (interactable.enemy_entity2 != 0) {
					auto &enemy = world.get_component<EnemyData>(interactable.enemy_entity2);
					enemy.is_blinded = false;
					break;
				}

				if (interactable.cable_parent != 0) {
					auto &cable = world.get_component<CableParent>(interactable.cable_parent);
					cable.color_value = -1.0f;
				}
			}
		}

		if (interactable.interaction == Interaction::HackerPlatform) {
			auto &platform = world.get_component<Platform>(interactable.interaction_targets[0]);
			if (platform.is_moving) {
				interactable.triggered = false;
				return;
			}
		}

		if (interactable.triggered) {
			interactable.triggered = false;

			if (interactable.lever != 0) {
				interactable.is_on = !interactable.is_on;
				interactable.is_rotating = true;
			}

			if (interactable.cable_parent > 0) {
				auto &cable = world.get_component<CableParent>(interactable.cable_parent);

				// switch to the other state
				if (cable.state == CableState::ON) {
					cable.state = CableState::OFF;
				} else {
					cable.state = CableState::ON;
				}
			}

			switch (interactable.interaction) {
				case Interaction::NoInteraction:
					no_interaction(world, interactable, entity);
					break;
				case Interaction::HackerCameraJump:
					interactable.triggered = false;
					break;
				case Interaction::HackerPlatform: {
					for (unsigned int current_target : interactable.interaction_targets) {
						if (current_target == 0) {
							break;
						}
						auto &platform = world.get_component<Platform>(current_target);
						if (!platform.is_moving) {
							platform.is_moving = true;
							AudioManager::get().play_one_shot_3d(
									electric_interaction_event, world.get_component<Transform>(entity));
						}
						//SPDLOG_INFO("{}", "Hacker platform triggered");
					}
					break;
				}
				case Interaction::Exploding: {
					SPDLOG_INFO("Explosion triggered");
					explosion(world, interactable, entity);
					interactable.can_interact = false;
					break;
				}
				case Interaction::LightSwitch: {
					for (Entity current_light_entity : interactable.interaction_targets) {
						if (current_light_entity == 0) {
							break;
						}
						SPDLOG_INFO("Light switch triggered");
						switch_light(world, current_light_entity);
					}
				}

				case Interaction::TemporalLightSwitch: {
					std::vector<Entity> lights_to_switch(
							interactable.interaction_targets.begin(), interactable.interaction_targets.end());
					std::vector<Entity> enemies_to_blind{};

					enemies_to_blind.push_back(interactable.enemy_entity);
					enemies_to_blind.push_back(interactable.enemy_entity2);

					switch_light_temporal(world, lights_to_switch, interactable, dt, enemies_to_blind);
					interactable.is_powering_up = true;
				}
			}

			if (interactable.single_use) {
				interactable.can_interact = false;
			}
		}
	}
}

void InteractableSystem::no_interaction(World &world, Interactable &interactable, Entity entity) {
	SPDLOG_WARN("No interaction set for entity {}, turning off interactions on this object", entity);
	//interactable.can_interact = false;
}

void InteractableSystem::explosion(World &world, Interactable &interactable, Entity entity) {
	auto &box = world.get_component<ExplodingBox>(interactable.interaction_targets[0]);
	auto &t = world.get_component<Transform>(interactable.interaction_targets[0]);

	if (world.has_component<ParticleEmitter>(entity)) {
		world.get_component<ParticleEmitter>(entity).trigger_oneshot();
	}

	AudioManager::get().play_one_shot_3d(explostion_event, world.get_component<Transform>(entity));

	if (world.has_component<FMODEmitter>(entity)) {
		auto &sound = world.get_component<FMODEmitter>(entity);
		sound.event_instance->setParameterByName("buzz_volume", 0.0f);
		sound.event_instance->setParameterByName("electricity_volume", 0.0f);
	}

	ColliderSphere sphere;
	float larger_radius = box.distraction_radius;
	float smaller_radius = box.explosion_radius;
	if (larger_radius < smaller_radius) {
		float temp = larger_radius;
		larger_radius = smaller_radius;
		smaller_radius = temp;
	}
	sphere.radius = larger_radius;
	sphere.center = world.get_component<Transform>(interactable.interaction_targets[0]).get_global_position();
	auto colliders = PhysicsManager::get().overlap_sphere(world, sphere, "default");
	for (Entity entity : colliders) {
		if (world.has_component<EnemyData>(entity)) {
			auto enemy_data = world.get_component<EnemyData>(entity);
			float distance = glm::distance(sphere.center, world.get_component<Transform>(entity).get_global_position());
			if (distance < smaller_radius) {
				auto &enemy = world.get_component<EnemyData>(entity);
				enemy.state_machine.set_state("dying");
			} else if (distance < larger_radius) {
				//TODO: set enemy state as distracted
				auto &enemy = world.get_component<EnemyData>(entity);
				auto &enemy_transform = world.get_component<Transform>(entity);
				// if enemy is dead, don't change his state
				if (enemy.state_machine.get_current_state() != "dying") {
					enemy.state_machine.set_state("distracted");
					enemy.distraction_cooldown = box.distraction_time;
					auto target = t.get_global_position();
					// yeah, same as in prototype, they would float to the root of the box
					target.y = enemy_transform.get_global_position().y;
					enemy.distraction_target = target;
				}
				SPDLOG_INFO("{} is distracted", entity);
			}
		}
	}
}

void InteractableSystem::switch_rotator(World &world, Entity &light_entity) {
	if (world.has_component<Rotator>(light_entity)) {
		auto &rotator = world.get_component<Rotator>(light_entity);
		rotator.is_rotating = !rotator.is_rotating;
	}
}

void InteractableSystem::switch_light(World &world, Entity light_entity) {
	auto &light = world.get_component<Light>(light_entity);
	light.is_on = !light.is_on;

	switch_rotator(world, light_entity);
}

void InteractableSystem::switch_light_temporal(World &world, const std::vector<Entity> &light_entities,
		Interactable &interactable, float dt, const std::vector<Entity> &enemy_entities) {
	interactable.can_interact = false;
	interactable.is_on = false;
	interactable.is_powering_up = true;

	for (Entity light_entity : light_entities) {
		if (light_entity == 0) {
			break;
		}
		auto &light = world.get_component<Light>(light_entity);
		light.is_on = !light.is_on;

		switch_rotator(world, light_entity);
	}
	for (Entity enemy_entity : enemy_entities) {
		if (enemy_entity == 0) {
			break;
		}
		auto &enemy = world.get_component<EnemyData>(enemy_entity);
		enemy.is_blinded = true;
	}
}
