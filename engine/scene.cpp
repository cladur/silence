#include "scene.h"
#include "animation/ecs/animation_instance.h"
#include "components/exploding_box_component.h"
#include "display/display_manager.h"
#include "ecs/systems/interactable_system.h"
#include "ecs/systems/platform_system.h"
#include "ecs/world.h"
#include "editor/editor.h"
#include "managers/animation/ecs/animation_instance.h"
#include "physics/physics_manager.h"
#include "render/ecs/model_instance.h"
#include "render/ecs/skinned_model_instance.h"
#include "render/render_manager.h"

#include "animation/ecs/animation_system.h"
#include "animation/ecs/attachment_system.h"
#include "ecs/systems/agent_system.h"
#include "ecs/systems/collider_draw.h"
#include "ecs/systems/enemy_path_draw_system.h"
#include "ecs/systems/enemy_pathing.h"
#include "ecs/systems/enemy_system.h"
#include "ecs/systems/hacker_system.h"
#include "ecs/systems/isolated_entities_system.h"
#include "ecs/systems/root_parent_system.h"
#include "managers/physics/ecs/collision_system.h"
#include "managers/physics/ecs/physics_system.h"
#include "render/ecs/frustum_draw_system.h"
#include "render/ecs/light_render_system.h"
#include "render/ecs/render_system.h"
#include "render/ecs/skinned_render_system.h"

#define COLLISION_TEST_ENTITY 4

Scene::Scene() {
	ZoneNamedNC(Zone1, "Scene::Scene()", tracy::Color::RebeccaPurple, true);
	// ECS
	world.startup();
	world.parent_scene = this;

	ZoneNamedNC(Zone2, "Scene::Scene()::Components", tracy::Color::Green, true);
	{
		world.register_component<Name>();
		world.register_component<Transform>();
		world.register_component<RigidBody>();
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
		world.register_component<ColliderCapsule>();
		world.register_component<ColliderOBB>();
		world.register_component<Light>();
		world.register_component<AgentData>();
		world.register_component<HackerData>();
		world.register_component<EnemyPath>();
		world.register_component<Interactable>();
		world.register_component<Attachment>();
		world.register_component<Platform>();
		world.register_component<ExplodingBox>();
		world.register_component<EnemyData>();
	}
	// Components

	ZoneNamedNC(Zone3, "Scene::Scene()::Systems", tracy::Color::Orange, true);
	{
		// Systems
		// TODO: Set update order instead of using default value
		world.register_system<RenderSystem>();
		world.register_system<SkinnedRenderSystem>();
		world.register_system<ColliderDrawSystem>();
		world.register_system<AnimationSystem>(EcsOnLoad);
		world.register_system<FrustumDrawSystem>();
		world.register_system<LightRenderSystem>();
		world.register_system<EnemyPathDraw>();

		// Transform
		world.register_system<IsolatedEntitiesSystem>(EcsOnLoad);
		world.register_system<RootParentSystem>(EcsOnLoad);
		world.register_system<AttachmentSystem>(EcsPostLoad);
	}

	auto &physics_manager = PhysicsManager::get();
	physics_manager.add_collision_layer("default");
	physics_manager.add_collision_layer("hacker");
	physics_manager.add_collision_layer("agent");
	physics_manager.set_layers_no_collision("default", "hacker");
	physics_manager.set_layers_no_collision("agent", "hacker");
}

void Scene::register_game_systems() {
	// Physics
	world.register_system<PhysicsSystem>(EcsOnUpdate);
	world.register_system<CollisionSystem>(EcsOnUpdate);

	// Agents
	world.register_system<AgentSystem>(EcsOnUpdate);
	world.register_system<HackerSystem>(EcsOnUpdate);
	world.register_system<EnemySystem>(EcsOnUpdate);
	//world.register_system<EnemyPathing>(EcsOnUpdate);
	world.register_system<InteractableSystem>(EcsOnUpdate);
	world.register_system<PlatformSystem>(EcsOnUpdate);
}

void Scene::update(float dt) {
	ZoneScopedN("Scene::update");
	for (Entity entity : entities) {
		if (world.has_component<Camera>(entity) && world.has_component<Transform>(entity)) {
			auto &camera = world.get_component<Camera>(entity);
			auto &transform = world.get_component<Transform>(entity);
			if (camera.right_side) {
				get_render_scene().right_camera_params = camera;
				get_render_scene().right_camera_transform = transform;
			} else {
				get_render_scene().left_camera_params = camera;
				get_render_scene().left_camera_transform = transform;
			}
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
}