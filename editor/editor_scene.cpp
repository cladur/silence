#include "editor_scene.h"
#include "engine/scene.h"

#include "display/display_manager.h"
#include "input/input_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "physics/physics_manager.h"

#include "editor.h"

void handle_camera(DebugCamera &cam, float dt) {
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
	InputManager &input_manager = InputManager::get();
	// bsp_tree = BSPSystem::build_tree(world, entities, 10);
}

void EditorScene::update(float dt) {
	Scene::update(dt);

	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();

	Editor::get()->viewport_hovered |= viewport_hovered;

	// Handle camera movement
	if ((viewport_hovered && input_manager.is_action_pressed("control_camera") || controlling_camera)) {
		controlling_camera = true;
		Editor::get()->controlling_camera = true;
		handle_camera(get_render_scene().debug_camera, dt);
		display_manager.capture_mouse(true);
	}

	if (input_manager.is_action_just_released("control_camera")) {
		controlling_camera = false;
		Editor::get()->controlling_camera = false;
		display_manager.capture_mouse(false);
	}
}

void EditorScene::duplicate_selected_entity() {
	if (selected_entity == 0) {
		return;
	}

	Entity parent = 0;
	if (world.has_component<Parent>(selected_entity)) {
		parent = world.get_component<Parent>(selected_entity).parent;
	}

	nlohmann::json scene_json = nlohmann::json::array();
	scene_json.push_back(nlohmann::json::object());
	world.serialize_entity_json(scene_json.back(), selected_entity);

	scene_json.back()["entity"] = 0;

	world.deserialize_entity_json(scene_json.back(), entities);

	selected_entity = entities.back();

	if (parent) {
		world.add_child(parent, selected_entity);
	}
}

void EditorScene::save_to_file(const std::string &path) {
	Scene::save_to_file(path);
}