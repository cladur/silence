#include "detection_camera_system.h"
#include "ai/state_machine/states/enemy/enemy_utils.h"
#include "components/detection_camera_component.h"
#include "ecs/systems/detection_camera_system.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include "gameplay/gameplay_manager.h"
#include <spdlog/spdlog.h>
#include <glm/common.hpp>
#include <glm/ext/matrix_transform.hpp>

AutoCVarFloat cvar_camera_friendly_time(
		"enemy_camera.friendly_time", "Time before camera starts detecting", 1.0f, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_enemy_camera_detection_range(
		"enemy_camera.detection_range", "Detection Camera Range", 12, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_enemy_camera_detection_angle(
		"enemy_camera.detection_angle", "Detection Camera Angle", 50, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_enemy_camera_detection_vertical_angle(
		"enemy_camera.detection_vertical_angle", "Detection Camera Vertical Angle", 135, CVarFlags::EditFloatDrag);

#define IDLE_COLOR_BEGIN glm::vec4(0.65f, 0.2f, 0.0f, 0.8f)
#define IDLE_COLOR_END glm::vec4(0.65f, 0.2f, 0.0f, 0.0f)

#define DETECTING_COLOR_BEGIN glm::vec4(1.0f, 0.0f, 0.0f, 0.8)
#define DETECTING_COLOR_END glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)

#define IDLE_COLOR glm::vec4(0.65f, 0.2f, 0.0f, 1.0f)
#define DETECTING_COLOR glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)

#define FRIENDLY_COLOR glm::vec4(0.0f, 1.0f, 0.0f, 0.8f)

#define IDLE_VELOCITY glm::vec3(0.0f, -5.0f, 0.0f)
#define TAGGED_VELOCITY glm::vec3(0.0f, -12.0f, 0.0f)

#define IDLE_VELOCITY_VARIANCE glm::vec3(5.0f, 0.0f, 5.0f)
#define TAGGED_VELOCITY_VARIANCE glm::vec3(9.0f, 0.0f, 9.0f)

#define IDLE_LIFETIME 0.5f
#define TAGGED_LIFETIME 5.0f

#define DETECTING_LIFETIME 2.5f

#define IDLE_SIZE 4.5f
#define TAGGED_SIZE 6.0f

void DetectionCameraSystem::startup(World &world) {
	Signature white_signature;
	white_signature.set(world.get_component_type<DetectionCamera>());
	world.set_system_component_whitelist<DetectionCameraSystem>(white_signature);
}

void DetectionCameraSystem::update(World &world, float dt) {
	ZoneScopedN("DetectionCameraSystem::update");

	for (auto const &entity : entities) {
		auto &detection_camera = world.get_component<DetectionCamera>(entity);
		auto &transform = world.get_component<Transform>(entity);
		auto &circle_billboard = world.get_component<Billboard>(entity);
		auto &light = world.get_component<Light>(detection_camera.camera_light);

		auto &tag = world.get_component<Taggable>(entity);

		Children *particle_children = nullptr;
		ParticleEmitter *particle_1 = nullptr;
		ParticleEmitter *particle_2 = nullptr;

		if (detection_camera.particles_parent != 0) {
			particle_children = &world.get_component<Children>(detection_camera.particles_parent);
			// we have two systems because one is vertical and the other is horizontal so the particles are visible from
			// all directions
			particle_1 = &world.get_component<ParticleEmitter>(particle_children->children[0]);
			particle_2 = &world.get_component<ParticleEmitter>(particle_children->children[1]);
		}

		if (detection_camera.first_frame) {
			auto &rm = ResourceManager::get();

			detection_camera.first_frame = false;
			auto &ui = UIManager::get();
			ui.create_ui_scene(std::to_string(entity) + "_detection");
			ui.activate_ui_scene(std::to_string(entity) + "_detection");

			auto &agent_anchor = ui.add_ui_anchor(std::to_string(entity) + "_detection", "agent_anchor");
			agent_anchor.x = 0.25f;
			agent_anchor.y = 0.5f;
			agent_anchor.is_screen_space = true;
			agent_anchor.display = true;
			ui.add_as_root(std::to_string(entity) + "_detection", "agent_anchor");
			auto &hacker_anchor = ui.add_ui_anchor(std::to_string(entity) + "_detection", "hacker_anchor");
			hacker_anchor.x = 0.75f;
			hacker_anchor.y = 0.5f;
			hacker_anchor.is_screen_space = true;
			hacker_anchor.display = true;
			ui.add_as_root(std::to_string(entity) + "_detection", "hacker_anchor");

			auto &agent_detection_outline = ui.add_ui_image(std::to_string(entity) + "_detection",
															 "agent_detection_outline");
			agent_detection_outline.position = glm::vec3(0.0f, 0.0f, 0.0f);
			agent_detection_outline.is_billboard = false;
			agent_detection_outline.is_screen_space = true;
			agent_detection_outline.size = glm::vec2(50.0f, 50.0f);
			agent_detection_outline.color = glm::vec4(1.0f);
			agent_detection_outline.texture = rm.load_texture(asset_path("detection_triangle_outline.ktx2").c_str());
			ui.add_to_root(std::to_string(entity) + "_detection", "agent_detection_outline", "agent_anchor");
			auto &hacker_detection_outline = ui.add_ui_image(std::to_string(entity) + "_detection",
															  "hacker_detection_outline");
			hacker_detection_outline.position = glm::vec3(0.0f, 0.0f, 0.0f);
			hacker_detection_outline.is_billboard = false;
			hacker_detection_outline.is_screen_space = true;
			hacker_detection_outline.size = glm::vec2(50.0f, 50.0f);
			hacker_detection_outline.color = glm::vec4(1.0f);
			hacker_detection_outline.texture = rm.load_texture(asset_path("detection_triangle_outline.ktx2").c_str());
			ui.add_to_root(std::to_string(entity) + "_detection", "hacker_detection_outline", "hacker_anchor");
			auto &agent_detection_fill = ui.add_ui_image(std::to_string(entity) + "_detection",
														  "agent_detection_fill");
			agent_detection_fill.position = glm::vec3(0.0f, 0.0f, 0.0f);
			agent_detection_fill.is_billboard = false;
			agent_detection_fill.is_screen_space = true;
			agent_detection_fill.size = glm::vec2(0.0f, 0.0f);
			agent_detection_fill.color = glm::vec4(1.0f);
			agent_detection_fill.texture = rm.load_texture(asset_path("detection_triangle_fill.ktx2").c_str());
			ui.add_to_root(std::to_string(entity) + "_detection", "agent_detection_fill", "agent_anchor");
			auto &hacker_detection_fill = ui.add_ui_image(std::to_string(entity) + "_detection",
														   "hacker_detection_fill");
			hacker_detection_fill.position = glm::vec3(0.0f, 0.0f, 0.0f);
			hacker_detection_fill.is_billboard = false;
			hacker_detection_fill.is_screen_space = true;
			hacker_detection_fill.size = glm::vec2(0.0f, 0.0f);
			hacker_detection_fill.color = glm::vec4(1.0f);
			hacker_detection_fill.texture = rm.load_texture(asset_path("detection_triangle_fill.ktx2").c_str());
			ui.add_to_root(std::to_string(entity) + "_detection", "hacker_detection_fill", "hacker_anchor");

			auto &agent_detection_screen_flash = ui.add_ui_image(std::to_string(entity) + "_detection",
					"agent_detection_screen_flash");
			agent_detection_screen_flash.position = glm::vec3(0.0f, 0.0f, 0.0f);
			agent_detection_screen_flash.is_billboard = false;
			agent_detection_screen_flash.is_screen_space = true;
			agent_detection_screen_flash.size = glm::vec2(1.0f, 1.0f);
			agent_detection_screen_flash.color = glm::vec4(1.0f);
			agent_detection_screen_flash.texture = rm.load_texture(asset_path("detection_overlay.ktx2").c_str());
			agent_detection_screen_flash.display = false;
			ui.add_to_root(std::to_string(entity) + "_detection", "agent_detection_screen_flash", "agent_anchor");

			auto &hacker_detection_screen_flash = ui.add_ui_image(std::to_string(entity) + "_detection",
					"hacker_detection_screen_flash");
			hacker_detection_screen_flash.position = glm::vec3(0.0f, 0.0f, 0.0f);
			hacker_detection_screen_flash.is_billboard = false;
			hacker_detection_screen_flash.is_screen_space = true;
			hacker_detection_screen_flash.size = glm::vec2(1.0f, 1.0f);
			hacker_detection_screen_flash.color = glm::vec4(1.0f);
			hacker_detection_screen_flash.texture = rm.load_texture(asset_path("detection_overlay.ktx2").c_str());
			hacker_detection_screen_flash.display = false;
			ui.add_to_root(std::to_string(entity) + "_detection", "hacker_detection_screen_flash", "hacker_anchor");


			detection_camera.starting_orientation =
					world.get_component<Transform>(detection_camera.camera_model).get_orientation();
			detection_camera.detection_event = AudioManager::get().create_event_instance("SFX/camera_detecting");

			// particles setup
			particle_1->color_begin = IDLE_COLOR_BEGIN;
			particle_1->color_end = IDLE_COLOR_END;
			particle_1->velocity_begin = IDLE_VELOCITY;
			particle_1->velocity_end = IDLE_VELOCITY;
			particle_1->velocity_variance = IDLE_VELOCITY_VARIANCE;
			particle_1->lifetime = IDLE_LIFETIME;
			particle_1->size_begin = IDLE_SIZE;
			particle_1->size_end = IDLE_SIZE;
			particle_1->rate = 6.0f;

			particle_2->color_begin = IDLE_COLOR_BEGIN;
			particle_2->color_end = IDLE_COLOR_END;
			particle_2->velocity_begin = IDLE_VELOCITY;
			particle_2->velocity_end = IDLE_VELOCITY;
			particle_2->velocity_variance = IDLE_VELOCITY_VARIANCE;
			particle_2->lifetime = IDLE_LIFETIME;
			particle_2->size_begin = IDLE_SIZE;
			particle_2->size_end = IDLE_SIZE;
			particle_2->rate = 6.0f;
		}

		if (tag.tagged) {
			particle_1->velocity_begin = TAGGED_VELOCITY;
			particle_1->velocity_end = TAGGED_VELOCITY;
			particle_1->velocity_variance = TAGGED_VELOCITY_VARIANCE;
			particle_1->lifetime = TAGGED_LIFETIME;
			particle_1->size_begin = TAGGED_SIZE;
			particle_1->size_end = TAGGED_SIZE;
			particle_1->rate = 6.0f;

			particle_2->velocity_begin = TAGGED_VELOCITY;
			particle_2->velocity_end = TAGGED_VELOCITY;
			particle_2->velocity_variance = TAGGED_VELOCITY_VARIANCE;
			particle_2->lifetime = TAGGED_LIFETIME;
			particle_2->size_begin = TAGGED_SIZE;
			particle_2->size_end = TAGGED_SIZE;
			particle_2->rate = 6.0f;
		} else {
			particle_1->velocity_begin = IDLE_VELOCITY;
			particle_1->velocity_end = IDLE_VELOCITY;
			particle_1->velocity_variance = IDLE_VELOCITY_VARIANCE;
			particle_1->lifetime = IDLE_LIFETIME;
			particle_1->size_begin = IDLE_SIZE;
			particle_1->size_end = IDLE_SIZE;
			particle_1->rate = 6.0f;

			particle_2->velocity_begin = IDLE_VELOCITY;
			particle_2->velocity_end = IDLE_VELOCITY;
			particle_2->velocity_variance = IDLE_VELOCITY_VARIANCE;
			particle_2->lifetime = IDLE_LIFETIME;
			particle_2->size_begin = IDLE_SIZE;
			particle_2->size_end = IDLE_SIZE;
			particle_2->rate = 6.0f;
		}

		if (detection_camera.is_detecting) {
			light.color = DETECTING_COLOR;

			particle_1->color_begin = DETECTING_COLOR_BEGIN;
			particle_1->color_end = DETECTING_COLOR_END;
			particle_1->lifetime = tag.tagged == true ? TAGGED_LIFETIME : DETECTING_LIFETIME;
			particle_1->velocity_begin = TAGGED_VELOCITY;
			particle_1->velocity_end = TAGGED_VELOCITY;

			particle_2->color_begin = DETECTING_COLOR_BEGIN;
			particle_2->color_end = DETECTING_COLOR_END;
			particle_2->lifetime = tag.tagged == true ? TAGGED_LIFETIME : DETECTING_LIFETIME;
			particle_2->velocity_begin = TAGGED_VELOCITY;
			particle_2->velocity_end = TAGGED_VELOCITY;

		} else {
			light.color = IDLE_COLOR;

			particle_1->color_begin = IDLE_COLOR_BEGIN;
			particle_1->color_end = IDLE_COLOR_END;
			particle_1->lifetime = tag.tagged == true ? TAGGED_LIFETIME : IDLE_LIFETIME;

			particle_2->color_begin = IDLE_COLOR_BEGIN;
			particle_2->color_end = IDLE_COLOR_END;
			particle_2->lifetime = tag.tagged == true ? TAGGED_LIFETIME : IDLE_LIFETIME;
		}

		enemy_utils::update_detection_slider_camera(entity, transform, detection_camera, world.get_parent_scene()->get_render_scene(), world.get_parent_scene());
		enemy_utils::handle_highlight(entity, &world);

		if (!detection_camera.is_active) {
			if (detection_camera.is_playing) {
				AudioManager::get().stop_local(detection_camera.detection_event);
				detection_camera.is_playing = false;
			}
			if (detection_camera.friendly_time_left > 0.0f) {
				detection_camera.friendly_time_left -= dt;
				detection_camera.friendly_time_left = glm::max(0.0f, detection_camera.friendly_time_left);
				if (detection_camera.friendly_time_left == 0.0f) {
					detection_camera.is_active = true;
				}

				glm::vec4 new_color = glm::mix(FRIENDLY_COLOR, IDLE_COLOR,
						1.0f - (detection_camera.friendly_time_left / cvar_camera_friendly_time.get()));

				light.color = new_color;
				particle_1->color_begin = new_color;
				particle_1->color_end = new_color;
				particle_2->color_begin = new_color;
				particle_2->color_end = new_color;

				circle_billboard.color = glm::mix(FRIENDLY_COLOR, DETECTING_COLOR,
						1.0f - (detection_camera.friendly_time_left / cvar_camera_friendly_time.get()));
			} else {
				light.color = FRIENDLY_COLOR;
				circle_billboard.color = FRIENDLY_COLOR;
				particle_1->color_begin = glm::vec4(0.0f);
				particle_1->color_end = glm::vec4(0.0f);
				particle_2->color_begin = glm::vec4(0.0f);
				particle_2->color_end = glm::vec4(0.0f);
			}
			return;
		}

		DebugDraw &debug_draw = world.get_parent_scene()->get_render_scene().debug_draw;
		auto &camera_model_transform = world.get_component<Transform>(detection_camera.camera_model);
		glm::vec3 forward = camera_model_transform.get_global_forward();
		enemy_utils::handle_detection_camera(
				&world, entity, camera_model_transform, -forward, detection_camera, dt, &debug_draw);
		update_light_color(world, detection_camera);
	}
}

void DetectionCameraSystem::update_light_color(World &world, DetectionCamera &detection_camera) {
	if (detection_camera.camera_light == 0) {
		return;
	}

	auto camera_light = world.get_component<Light>(detection_camera.camera_light);
	if (detection_camera.detection_level == 0.0f) {
		camera_light.color = IDLE_COLOR;
	} else {
		camera_light.color = glm::mix(IDLE_COLOR, DETECTING_COLOR, detection_camera.detection_level);
	}
}