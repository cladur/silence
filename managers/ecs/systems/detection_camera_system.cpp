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

#define IDLE_START_VELOCITY glm::vec3(0.0f, -0.6f, 0.0f)
#define IDLE_END_VELOCITY glm::vec3(0.0f, -0.2f, 0.0f)
#define IDLE_LIFETIME 3.0f
#define IDLE_VELOCITY_VARIANCE glm::vec3(0.5f, 0.0f, 0.5f)
#define IDLE_RATE 15.0f

#define DETECTING_START_VELOCITY glm::vec3(0.0f, -3.0f, 0.0f)
#define DETECTING_END_VELOCITY glm::vec3(0.0f, -1.0f, 0.0f)
#define DETECTING_LIFETIME 1.0f
#define DETECTING_VELOCITY_VARIANCE glm::vec3(2.5f, 0.0f, 2.5f)
#define DETECTING_RATE 45.0f

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

		enemy_utils::update_detection_slider_camera(entity, transform, detection_camera);
		enemy_utils::handle_highlight(entity, &world);

		if (detection_camera.first_frame) {
			detection_camera.first_frame = false;
			auto &ui = UIManager::get();
			ui.create_ui_scene(std::to_string(entity) + "_detection");
			auto &slider = ui.add_ui_slider(std::to_string(entity) + "_detection", "detection_slider");
			slider.position = glm::vec3(0.0f, 2.0f, 0.0f);
			slider.is_billboard = true;
			slider.is_screen_space = false;
			slider.size = glm::vec2(0.1f, 0.5f);
			slider.slider_alignment = SliderAlignment::BOTTOM_TO_TOP;
			slider.color = glm::vec4(1.0f);

			ui.add_as_root(std::to_string(entity) + "_detection", "detection_slider");
			ui.activate_ui_scene(std::to_string(entity) + "_detection");

			detection_camera.starting_orientation =
					world.get_component<Transform>(detection_camera.camera_model).get_orientation();
			detection_camera.detection_event = AudioManager::get().create_event_instance("SFX/camera_detecting");
		}

		if (detection_camera.is_detecting) {
			light.color = DETECTING_COLOR;

			particle_1->color_begin = DETECTING_COLOR_BEGIN;
			particle_1->color_end = DETECTING_COLOR_END;
			particle_1->velocity_begin = DETECTING_START_VELOCITY;
			particle_1->velocity_end = DETECTING_END_VELOCITY;
			particle_1->lifetime = DETECTING_LIFETIME;
			particle_1->velocity_variance = DETECTING_VELOCITY_VARIANCE;
			particle_1->rate = DETECTING_RATE;

			particle_2->color_begin = DETECTING_COLOR_BEGIN;
			particle_2->color_end = DETECTING_COLOR_END;
			particle_2->velocity_begin = DETECTING_START_VELOCITY;
			particle_2->velocity_end = DETECTING_END_VELOCITY;
			particle_2->lifetime = DETECTING_LIFETIME;
			particle_2->velocity_variance = DETECTING_VELOCITY_VARIANCE;
			particle_2->rate = DETECTING_RATE;

		} else {
			light.color = IDLE_COLOR;

			particle_1->color_begin = IDLE_COLOR_BEGIN;
			particle_1->color_end = IDLE_COLOR_END;
			particle_1->velocity_begin = IDLE_START_VELOCITY;
			particle_1->velocity_end = IDLE_END_VELOCITY;
			particle_1->lifetime = IDLE_LIFETIME;
			particle_1->velocity_variance = IDLE_VELOCITY_VARIANCE;
			particle_1->rate = IDLE_RATE;

			particle_2->color_begin = IDLE_COLOR_BEGIN;
			particle_2->color_end = IDLE_COLOR_END;
			particle_2->velocity_begin = IDLE_START_VELOCITY;
			particle_2->velocity_end = IDLE_END_VELOCITY;
			particle_2->lifetime = IDLE_LIFETIME;
			particle_2->velocity_variance = IDLE_VELOCITY_VARIANCE;
			particle_2->rate = IDLE_RATE;
		}

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