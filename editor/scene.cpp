#include "scene.h"
#include "display/display_manager.h"
#include "ecs/ecs_manager.h"
#include "editor.h"
#include "input/input_manager.h"
#include "render/ecs/model_instance.h"
#include "render/render_manager.h"

void handle_camera(Camera &cam, float dt) {
	InputManager &input_manager = InputManager::get();
	float forward = input_manager.get_axis("move_backward", "move_forward");
	float right = input_manager.get_axis("move_left", "move_right");
	float up = input_manager.get_axis("move_down", "move_up");

	if (input_manager.is_action_pressed("move_faster")) {
		dt *= 3.0f;
	}

	cam.move_forward(forward * dt);
	cam.move_right(right * dt);
	cam.move_up(up * dt);

	glm::vec2 mouse_delta = input_manager.get_mouse_delta();
	cam.rotate(mouse_delta.x * dt, mouse_delta.y * dt);
}

Scene::Scene() {
	camera = Camera(glm::vec3(-4.0f, 2.6f, -4.0f));
	camera.yaw = 45.0f;
	camera.pitch = -20.0f;
	camera.update_camera_vectors();
}

void Scene::update(float dt) {
	ECSManager &ecs_manager = ECSManager::get();
	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();

	Editor::get()->viewport_hovered |= viewport_hovered;

	get_render_scene().camera = camera;

	get_render_scene().debug_draw.draw_line(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));
	get_render_scene().debug_draw.draw_line(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 0.0f, 0.0f));

	get_render_scene().debug_draw.draw_line(
			glm::vec3(0.0f, 5.0f, 10.0f), glm::vec3(10.0f, 0.0f, 2.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	get_render_scene().debug_draw.draw_box(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
	get_render_scene().debug_draw.draw_box(
			glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(10.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	// Handle camera movement
	if ((viewport_hovered && input_manager.is_action_pressed("control_camera") || controlling_camera)) {
		controlling_camera = true;
		Editor::get()->controlling_camera = true;
		handle_camera(camera, dt);
		display_manager.capture_mouse(true);
	}

	if (input_manager.is_action_just_released("control_camera")) {
		controlling_camera = false;
		Editor::get()->controlling_camera = false;
		display_manager.capture_mouse(false);
	}

	for (auto &entity : entities) {
		if (ecs_manager.has_component<Transform>(entity) && ecs_manager.has_component<ModelInstance>(entity)) {
			auto &transform = ecs_manager.get_component<Transform>(entity);
			auto &model_instance = ecs_manager.get_component<ModelInstance>(entity);

			get_render_scene().queue_draw(&model_instance, &transform);
		}
	}
}
RenderScene &Scene::get_render_scene() {
	RenderManager &render_manager = RenderManager::get();
	return render_manager.render_scenes[render_scene_idx];
}
