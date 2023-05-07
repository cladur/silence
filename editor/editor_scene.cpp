#include "editor_scene.h"
#include "engine/scene.h"

#include "display/display_manager.h"
#include "input/input_manager.h"

#include "editor.h"

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

EditorScene::EditorScene(SceneType type) {
	this->type = type;
	multi_select_parent = world.create_entity();
	world.add_component<Transform>(multi_select_parent, Transform());
}

void EditorScene::update(float dt) {
	Scene::update(dt);

	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();

	Editor::get()->viewport_hovered |= viewport_hovered;

	auto &trans = world.get_component<Transform>(multi_select_parent);

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
}

void EditorScene::save_to_file(const std::string &path) {
	clear_selection();
	Scene::save_to_file(path);
}

void EditorScene::add_to_selection(Entity entity) {
	if (world.has_component<Parent>(entity)) {
		Entity old_parent = world.get_component<Parent>(entity).parent;
		child_to_parent[entity] = old_parent;
		reparent_queue.emplace_back(multi_select_parent, entity);
	} else {
		child_to_parent[entity] = 0;
		add_child_queue.emplace_back(multi_select_parent, entity);
	}
	entities_selected.push_back(entity);
	last_entity_selected = entity;
}

void EditorScene::remove_from_selection(Entity entity) {
	if (child_to_parent.find(entity) != child_to_parent.end()) {
		Entity old_parent = child_to_parent[entity];
		if (old_parent == 0) {
			if (world.has_component<Parent>(entity)) {
				world.remove_child(multi_select_parent, entity, true);
			}
		} else {
			// world.reparent(old_parent, entity);
			reparent_queue.emplace_back(old_parent, entity);
		}
		child_to_parent.erase(entity);
	}
	entities_selected.erase(
			std::remove(entities_selected.begin(), entities_selected.end(), entity), entities_selected.end());
}

void EditorScene::clear_selection() {
	while (!entities_selected.empty()) {
		remove_from_selection(entities_selected[0]);
	}
	execute_reparent_queue();
}

void EditorScene::calculate_multi_select_parent() {
	auto &transform = world.get_component<Transform>(multi_select_parent);
	if (entities_selected.size() <= 1) {
		transform.set_position(glm::vec3(0.0f));
		transform.set_euler_rot(glm::vec3(0.0f));
		transform.set_scale(glm::vec3(1.0f));
		transform.update_global_model_matrix();
		return;
	}
	auto average_pos = glm::vec3(0.0f);
	for (auto &entity : entities_selected) {
		if (world.has_component<Transform>(entity)) {
			auto &transform = world.get_component<Transform>(entity);
			average_pos += transform.get_global_position();
		}
	}
	average_pos /= entities_selected.size();

	transform.set_position(average_pos);
	if (entities_selected.size() == 1) {
		auto &t = world.get_component<Transform>(entities_selected[0]);
		transform.set_euler_rot(t.get_global_euler_rot());
		transform.set_scale(t.get_scale());
	} else {
		transform.set_euler_rot(glm::vec3(0.0f));
		transform.set_scale(glm::vec3(1.0f));
	}

	transform.update_global_model_matrix();

	for (auto &entity : entities_selected) {
		if (world.has_component<Transform>(entity)) {
			auto &t = world.get_component<Transform>(entity);
			t.reparent_to(transform);
		}
	}

	dummy_transform = transform;
}

void EditorScene::execute_reparent_queue() {
	for (auto &pair : reparent_queue) {
		world.reparent(pair.first, pair.second, true);
	}
	reparent_queue.clear();
	for (auto &pair : add_child_queue) {
		world.add_child(pair.first, pair.second, true);
	}
	add_child_queue.clear();
}
