#include "detection_camera_system.h"
#include "ai/state_machine/states/enemy/enemy_utils.h"
#include "components/detection_camera_component.h"
#include "ecs/world.h"
#include "engine/scene.h"

AutoCVarFloat cvar_enemy_camera_detection_range(
		"enemy_camera.detection_range", "Detection Camera Range", 12, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_enemy_camera_detection_angle(
		"enemy_camera.detection_angle", "Detection Camera Angle", 50, CVarFlags::EditFloatDrag);
AutoCVarFloat cvar_enemy_camera_detection_vertical_angle(
		"enemy_camera.detection_vertical_angle", "Detection Camera Vertical Angle", 135, CVarFlags::EditFloatDrag);

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
		}

		if (!detection_camera.is_active) {
			return;
		}
		DebugDraw &debug_draw = world.get_parent_scene()->get_render_scene().debug_draw;

		glm::vec3 forward = transform.get_global_forward();

		enemy_utils::handle_detection_camera(
				&world, entity, transform, -forward, detection_camera, dt, &debug_draw);
	}
}