#include "scene.h"
#include "display/display_manager.h"
#include "ecs/ecs_manager.h"
#include "editor.h"
#include "input/input_manager.h"
#include "render/ecs/model_instance.h"
#include "render/render_manager.h"
#include <unordered_map>

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

	ECSManager &ecs_manager = ECSManager::get();
	multi_select_parent = ecs_manager.create_entity();
	ecs_manager.add_component<Transform>(multi_select_parent, Transform());
}

void Scene::update(float dt) {
	ECSManager &ecs_manager = ECSManager::get();
	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();

	Editor::get()->viewport_hovered |= viewport_hovered;

	get_render_scene().camera = camera;

	auto &trans = ecs_manager.get_component<Transform>(multi_select_parent);

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

void Scene::add_to_selection(Entity entity) {
	ECSManager &ecs_manager = ECSManager::get();
	if (ecs_manager.has_component<Parent>(entity)) {
		Entity old_parent = ecs_manager.get_component<Parent>(entity).parent;
		child_to_parent[entity] = old_parent;
		reparent_queue.emplace_back(multi_select_parent, entity);
	} else {
		child_to_parent[entity] = 0;
		add_child_queue.emplace_back(multi_select_parent, entity);
	}
	entities_selected.push_back(entity);
	last_entity_selected = entity;
}

void Scene::remove_from_selection(Entity entity) {
	ECSManager &ecs_manager = ECSManager::get();
	if (child_to_parent.find(entity) != child_to_parent.end()) {
		Entity old_parent = child_to_parent[entity];
		if (old_parent == 0) {
			if (ecs_manager.has_component<Parent>(entity)) {
				ecs_manager.remove_child(multi_select_parent, entity, true);
			}
		} else {
			// ecs_manager.reparent(old_parent, entity);
			reparent_queue.emplace_back(old_parent, entity);
		}
		child_to_parent.erase(entity);
	}
	entities_selected.erase(
			std::remove(entities_selected.begin(), entities_selected.end(), entity), entities_selected.end());
}

void Scene::clear_selection() {
	while (!entities_selected.empty()) {
		remove_from_selection(entities_selected[0]);
	}
	execute_reparent_queue();
}

void Scene::calculate_multi_select_parent() {
	auto &transform = ECSManager::get().get_component<Transform>(multi_select_parent);
	if (entities_selected.size() <= 1) {
		transform.set_position(glm::vec3(0.0f));
		transform.set_euler_rot(glm::vec3(0.0f));
		transform.set_scale(glm::vec3(1.0f));
		transform.update_global_model_matrix();
		return;
	}
	auto average_pos = glm::vec3(0.0f);
	for (auto &entity : entities_selected) {
		if (ECSManager::get().has_component<Transform>(entity)) {
			auto &transform = ECSManager::get().get_component<Transform>(entity);
			average_pos += transform.get_global_position();
		}
	}
	average_pos /= entities_selected.size();

	transform.set_position(average_pos);
	if (entities_selected.size() == 1) {
		auto &t = ECSManager::get().get_component<Transform>(entities_selected[0]);
		transform.set_euler_rot(t.get_global_euler_rot());
		transform.set_scale(t.get_scale());
	} else {
		transform.set_euler_rot(glm::vec3(0.0f));
		transform.set_scale(glm::vec3(1.0f));
	}

	transform.update_global_model_matrix();

	for (auto &entity : entities_selected) {
		if (ECSManager::get().has_component<Transform>(entity)) {
			auto &t = ECSManager::get().get_component<Transform>(entity);
			t.reparent_to(transform);
		}
	}

	dummy_transform = transform;
}

void Scene::execute_reparent_queue() {
	ECSManager &ecs_manager = ECSManager::get();
	for (auto &pair : reparent_queue) {
		ecs_manager.reparent(pair.first, pair.second, true);
	}
	reparent_queue.clear();
	for (auto &pair : add_child_queue) {
		ecs_manager.add_child(pair.first, pair.second, true);
	}
	add_child_queue.clear();
}