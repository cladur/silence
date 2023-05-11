#include "editor_scene.h"
#include "engine/scene.h"

#include "display/display_manager.h"
#include "input/input_manager.h"

#include "editor.h"

#include "ecs/systems/bsp_system.h"

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
	bsp_tree = BSPSystem::build_tree(world, entities, 10);
}

void EditorScene::update(float dt) {
	Scene::update(dt);
	bsp_tree = BSPSystem::build_tree(world, entities, 10);

	BSPNode *node = bsp_tree.get();
	std::vector<BSPNode> nodes;
	while (node != nullptr) {
		nodes.push_back(*node);
		if (node->front != nullptr) {
			node = node->front.get();
		} else if (node->back != nullptr) {
			node = node->back.get();
		} else {
			node = nullptr;
		}
	}

	for (auto &node : nodes) {
		SPDLOG_INFO("{}", glm::to_string(node.plane.normal));
		get_render_scene().debug_draw.draw_box(
				node.plane.point, node.plane.normal, glm::vec3(20.0f, 20.0f, 0.1f), glm::vec3(1.0f, 0.0f, 0.0f));
	}

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

void EditorScene::save_to_file(const std::string &path) {
	Scene::save_to_file(path);
}