#include "scene.h"
#include "display/display_manager.h"
#include "ecs/world.h"
#include "editor/editor.h"
#include "render/ecs/animation_instance.h"
#include "render/ecs/model_instance.h"
#include "render/ecs/skinned_model_instance.h"
#include "render/render_manager.h"
#include <unordered_map>

#include "ecs/systems/bsp_system.h"
#include "ecs/systems/collider_draw.h"
#include "ecs/systems/collision_system.h"
#include "ecs/systems/parent_system.h"
#include "ecs/systems/physics_system.h"
#include "render/ecs/animation_system.h"
#include "render/ecs/frustum_draw_system.h"
#include "render/ecs/render_system.h"
#include "render/ecs/skinned_render_system.h"

#define COLLISION_TEST_ENTITY 4

Scene::Scene() {
	// ECS
	world.startup();
	world.parent_scene = this;

	// Components
	world.register_component<Name>();
	world.register_component<Transform>();
	world.register_component<RigidBody>();
	world.register_component<Gravity>();
	world.register_component<Parent>();
	world.register_component<Children>();
	world.register_component<ModelInstance>();
	world.register_component<SkinnedModelInstance>();
	world.register_component<AnimationInstance>();
	world.register_component<FmodListener>();
	world.register_component<Camera>();
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
	world.register_system<SkinnedRenderSystem>();
	world.register_system<ColliderDrawSystem>();
	world.register_system<AnimationSystem>();
	world.register_system<FrustumDrawSystem>();

	//todo uncomment if bspsystem is fixed
	// world.register_system<BSPSystem>();
}

void Scene::update(float dt) {
	for (Entity entity : entities) {
		if (world.has_component<Camera>(entity) && world.has_component<Transform>(entity)) {
			auto &camera = world.get_component<Camera>(entity);
			auto &transform = world.get_component<Transform>(entity);
			get_render_scene().camera_params = camera;
			get_render_scene().camera_transform = transform;
		}
	}
}

RenderScene &Scene::get_render_scene() const {
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

	bsp_tree = BSPSystem::build_tree(world, entities, 2);
}