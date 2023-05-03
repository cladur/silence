#include "scene.h"
#include "display/display_manager.h"
#include "ecs/world.h"
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

	// ECS
	world.startup();

	// Components
	world.register_component<Name>();
	world.register_component<Transform>();
	world.register_component<RigidBody>();
	world.register_component<Gravity>();
	world.register_component<Parent>();
	world.register_component<Children>();
	world.register_component<ModelInstance>();
	world.register_component<FmodListener>();
	world.register_component<ColliderTag>();
	world.register_component<ColliderSphere>();
	world.register_component<ColliderAABB>();
	world.register_component<ColliderOBB>();

	// Systems
	world.register_system<PhysicsSystem>();
	world.register_system<CollisionSystem>();
	world.register_system<ParentSystem>();
	world.register_system<RenderSystem>();

	multi_select_parent = world.create_entity();
	world.add_component<Transform>(multi_select_parent, Transform());
}

void Scene::update(float dt) {
	InputManager &input_manager = InputManager::get();
	DisplayManager &display_manager = DisplayManager::get();

	Editor::get()->viewport_hovered |= viewport_hovered;

	world.update(dt);

	get_render_scene().camera = camera;

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

	for (auto &entity : entities) {
		if (world.has_component<Transform>(entity) && world.has_component<ModelInstance>(entity)) {
			auto &transform = world.get_component<Transform>(entity);
			auto &model_instance = world.get_component<ModelInstance>(entity);

			get_render_scene().queue_draw(&model_instance, &transform);
		}
	}
}

RenderScene &Scene::get_render_scene() {
	RenderManager &render_manager = RenderManager::get();
	return render_manager.render_scenes[render_scene_idx];
}

void Scene::save_to_file(const std::string &path) {
	if (!path.empty()) {
		this->path = path;
	}
	if (this->path.empty()) {
		return;
	}
	clear_selection();
	auto json = SceneManager::save_scene(world, entities);
	SceneManager::save_json_to_file(this->path, json);
}

void Scene::load_from_file(const std::string &path) {
	if (path.empty()) {
		SPDLOG_ERROR("Wrong path: ", path, " should not be empty");
		assert(false);
	}
	this->path = path;
	if (this->path.empty()) {
		return;
	}
	std::ifstream file(this->path);
	if (!file.is_open()) {
		SPDLOG_WARN("Failed to open file: ", this->path);
		return;
	}
	nlohmann::json scene_json = nlohmann::json::parse(file);
	file.close();
	SceneManager::load_scene_from_json_file(world, scene_json, "", entities);
}

void Scene::add_to_selection(Entity entity) {
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

void Scene::remove_from_selection(Entity entity) {
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

void Scene::clear_selection() {
	while (!entities_selected.empty()) {
		remove_from_selection(entities_selected[0]);
	}
	execute_reparent_queue();
}

void Scene::calculate_multi_select_parent() {
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

void Scene::execute_reparent_queue() {
	for (auto &pair : reparent_queue) {
		world.reparent(pair.first, pair.second, true);
	}
	reparent_queue.clear();
	for (auto &pair : add_child_queue) {
		world.add_child(pair.first, pair.second, true);
	}
	add_child_queue.clear();
}