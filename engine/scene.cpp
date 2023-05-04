#include "scene.h"
#include "display/display_manager.h"
#include "ecs/world.h"
#include "editor/editor.h"
#include "input/input_manager.h"
#include "render/ecs/model_instance.h"
#include "render/render_manager.h"
#include <unordered_map>

#include "ecs/systems/collision_system.h"
#include "ecs/systems/parent_system.h"
#include "ecs/systems/physics_system.h"
#include "render/ecs/render_system.h"

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
	world.register_component<StaticTag>();
	world.register_component<ColliderTag>();
	world.register_component<ColliderSphere>();
	world.register_component<ColliderAABB>();
	world.register_component<ColliderOBB>();

	// Systems
	// TODO: Set update order instead of using default value
	world.register_system<PhysicsSystem>();
	world.register_system<CollisionSystem>();
	world.register_system<ParentSystem>();
	world.register_system<RenderSystem>();
}

void Scene::update(float dt) {
	world.update(dt);

	get_render_scene().camera = camera;

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