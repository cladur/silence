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

	input_manager.add_action("ray_cast");
	input_manager.add_key_to_action("ray_cast", InputKey::MOUSE_LEFT);
	// bsp_tree = BSPSystem::build_tree(world, entities, 10);
}

void EditorScene::update(float dt) {
	Scene::update(dt);

	// BSP Vizualization, for now it's left in here, but could be moved to system in the future
	//
	//	bsp_tree = CollisionSystem::build_tree(world, entities, 10);
	//
	//	BSPNode *node = bsp_tree.get();
	//	std::vector<BSPNode> nodes;
	//	while (node != nullptr) {
	//		nodes.push_back(*node);
	//		if (node->front != nullptr) {
	//			node = node->front.get();
	//		} else if (node->back != nullptr) {
	//			node = node->back.get();
	//		} else {
	//			node = nullptr;
	//		}
	//	}
	//
	//	for (auto &node : nodes) {
	//		// calculate rotation from normal
	//		glm::vec3 up = glm::vec3(0.0f, 0.0f, 1.0f);
	//		glm::vec3 axis = glm::cross(up, node.plane.normal);
	//		float angle = glm::acos(glm::dot(up, node.plane.normal));
	//		glm::quat rotation = glm::angleAxis(angle, axis);
	//
	//		if (node.front || node.back) {
	//			SPDLOG_INFO("normal {}", glm::to_string(node.plane.normal));
	//			SPDLOG_INFO("point {}", glm::to_string(node.plane.point));
	//			SPDLOG_INFO("entities {}", node.entities.size());
	//			get_render_scene().debug_draw.draw_box(
	//					node.plane.point, rotation, glm::vec3(20.0f, 20.0f, 0.1f), glm::vec3(1.0f, 0.0f, 0.0f));
	//		}
	//	}

	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();

	Editor::get()->viewport_hovered |= viewport_hovered;

	// Handle camera movement
	if ((viewport_hovered && input_manager.is_action_pressed("control_camera") || controlling_camera)) {
		controlling_camera = true;
		Editor::get()->controlling_camera = true;
		handle_camera(get_render_scene().debug_camera, dt);
		display_manager.capture_mouse(true);

		if (input_manager.is_action_pressed("ray_cast")) {
			Ray ray{};
			ray.origin = get_render_scene().debug_camera.get_position();
			ray.direction = glm::normalize(get_render_scene().debug_camera.get_front());
			glm::vec3 end = ray.origin + ray.direction * 1000.0f;
			get_render_scene().debug_draw.draw_arrow(ray.origin, end);
			HitInfo info;
			if (CollisionSystem::ray_cast(world, ray, info)) {
				SPDLOG_INFO("point {}", glm::to_string(info.point));
				SPDLOG_INFO("normal {}", glm::to_string(info.normal));
				SPDLOG_INFO("distance {}", info.distance);
				SPDLOG_INFO("entity {}", info.entity);
				if (world.has_component<Name>(info.entity)) {
					SPDLOG_INFO("name {}", world.get_component<Name>(info.entity).name);
				}
			}
		}
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
	nlohmann::json scene_json = nlohmann::json::array();
	scene_json.push_back(nlohmann::json::object());
	world.serialize_entity_json(scene_json.back(), selected_entity);

	// TODO: DO IT
}

void EditorScene::save_to_file(const std::string &path) {
	Scene::save_to_file(path);
}