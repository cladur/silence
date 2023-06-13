#include "detection_camera_system.h"
#include "ai/state_machine/states/enemy/enemy_utils.h"
#include "components/detection_camera_component.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include "gameplay/gameplay_manager.h"
#include <glm/ext/matrix_transform.hpp>

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

#define IDLE_VELOCITY glm::vec3(0.0f, -5.0f, 0.0f)
#define TAGGED_VELOCITY glm::vec3(0.0f, -17.0f, 0.0f)

#define IDLE_VELOCITY_VARIANCE glm::vec3(5.0f, 0.0f, 5.0f)
#define TAGGED_VELOCITY_VARIANCE glm::vec3(9.0f, 0.0f, 9.0f)

#define IDLE_LIFETIME 0.5f
#define DETECTING_LIFETIME 5.0f

#define IDLE_SIZE 4.5f
#define TAGGED_SIZE 8.0f

void DetectionCameraSystem::startup(World &world) {
	Signature white_signature;
	white_signature.set(world.get_component_type<DetectionCamera>());
	world.set_system_component_whitelist<DetectionCameraSystem>(white_signature);
}

void DetectionCameraSystem::update(World &world, float dt) {
	ZoneScopedN("DetectionCameraSystem::update");

	for (auto const &entity : entities) {
		auto &dc = world.get_component<DetectionCamera>(entity);
		auto &transform = world.get_component<Transform>(entity);

		auto &tag = world.get_component<Taggable>(entity);

		Children *particle_children = nullptr;
		ParticleEmitter *particle_1 = nullptr;
		ParticleEmitter *particle_2 = nullptr;

		if (dc.particles_parent != 0) {
			particle_children = &world.get_component<Children>(dc.particles_parent);
			// we have two systems because one is vertical and the other is horizontal so the particles are visible from all directions
			particle_1 = &world.get_component<ParticleEmitter>(particle_children->children[0]);
			particle_2 = &world.get_component<ParticleEmitter>(particle_children->children[1]);
		}


		enemy_utils::update_detection_slider_camera(entity, transform, dc);
		enemy_utils::handle_highlight(entity, &world);

		if (dc.first_frame) {
			dc.first_frame = false;
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

			dc.starting_orientation = transform.get_orientation();
			dc.detection_event = AudioManager::get().create_event_instance("SFX/camera_detecting");

			if (dc.particles_parent != 0) {
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

		}

		bool tagged = tag.tagged;

		if (tagged != dc.previous_frame_tag_state ) {
			dc.previous_frame_tag_state = tagged;

			if (dc.particles_parent != 0) {
				if (tagged) {
					particle_1->velocity_begin = TAGGED_VELOCITY;
					particle_1->velocity_end = TAGGED_VELOCITY;
					particle_1->velocity_variance = TAGGED_VELOCITY_VARIANCE;
					particle_1->lifetime = DETECTING_LIFETIME;
					particle_1->size_begin = TAGGED_SIZE;
					particle_1->size_end = TAGGED_SIZE;
					particle_1->rate = 6.0f;

					particle_2->velocity_begin = TAGGED_VELOCITY;
					particle_2->velocity_end = TAGGED_VELOCITY;
					particle_2->velocity_variance = TAGGED_VELOCITY_VARIANCE;
					particle_2->lifetime = DETECTING_LIFETIME;
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
			}
		}

		if (dc.particles_parent != 0) {
			if (dc.detection_level > 0.01f) {
				particle_1->color_begin = DETECTING_COLOR_BEGIN;
				particle_1->color_end = DETECTING_COLOR_END;

				particle_2->color_begin = DETECTING_COLOR_BEGIN;
				particle_2->color_end = DETECTING_COLOR_END;
			} else {
				particle_1->color_begin = IDLE_COLOR_BEGIN;
				particle_1->color_end = IDLE_COLOR_END;

				particle_2->color_begin = IDLE_COLOR_BEGIN;
				particle_2->color_end = IDLE_COLOR_END;
			}

			if (!dc.is_active) {
				particle_1->color_begin = glm::vec4(0.0f);
				particle_1->color_end = glm::vec4(0.0f);
				particle_2->color_begin = glm::vec4(0.0f);
				particle_2->color_end = glm::vec4(0.0f);
			}
		}

		if (!dc.is_active) {
			return;
		}

		DebugDraw &debug_draw = world.get_parent_scene()->get_render_scene().debug_draw;

		glm::vec3 forward = transform.get_global_forward();

		enemy_utils::handle_detection_camera(
				&world, entity, transform, -forward, dc, dt, &debug_draw);
	}
}